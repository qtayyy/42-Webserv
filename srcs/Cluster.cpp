/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 17:06:29 by qtay              #+#    #+#             */
/*   Updated: 2025/01/21 18:24:39 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"
#include "Log.hpp"

// ================================= DESTRUCTOR ===============================

/**
 * @brief	Destructor that closes all sockets and frees Client objects
 */
Cluster::~Cluster(void)
{
	for (int i = 0; i < _numOfFds; i++)
	{
		close(_pollFds[i].fd);
	}
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
		delete it->second;
}

// ============================= PARSE CONFIG FILE ============================

/**
 * @brief	Tokenizes the config file and parse it. Also maps each ip:port to
 * 			its possible server blocks.
 */
void	Cluster::parse(void)
{
	std::vector<string>	tokens;
	try
	{
		tokens = tokenizeConfig(this->_configPath);
		parseConfig(tokens);
		mapIPPortToServer();
		LogStream::success() << "Config file parsing complete.\n";
	}
	catch(const std::exception& e)
	{
		LogStream::error() << e.what() << '\n';
	}
}

// HELPER FUNCTION FOR CLUSTER::PARSE()
std::vector<string>	Cluster::tokenizeConfig(string &configPath)
{
	std::ifstream configFile(configPath.c_str());

	if (!configFile.is_open())
		throw ClusterException(RED + configPath + ": No such file or directory" RESET);
	string	line;
	std::vector<string>	tokens;
	while (std::getline(configFile, line))
	{
		line = strTrim(line, " \t");
		if (!line.empty() && line[0] == '#')
			continue ;
		if (!line.empty() && (line[line.size() - 1] != '{' && line[line.size() - 1] != '}') && line[line.size() - 1] != ';')
			throw ClusterException(RED "Config file error: missing semicolon" RESET);
		size_t	start = 0, end;
		while (start < line.size())
		{
			if ((end = line.find_first_of(" \t;", start)) == string::npos)
			{
				tokens.push_back(line.substr(start));
				break ;
			}
			if (end > start)
				tokens.push_back(line.substr(start, end - start));
			if (line[end] == ';')
				tokens.push_back(";");
			start = end + 1;
		}
	}
	configFile.close();
	return (tokens);
}

// HELPER FUNCTION FOR CLUSTER::PARSE()
void	Cluster::mapIPPortToServer(void)
{
	std::vector<std::pair<uint32_t, int> >	listens;
	std::stringstream	ss;
	string	IPPort;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		listens = _servers[i].getListen();
		for (size_t j = 0; j < listens.size(); j++)
		{
			ss.str("");
			ss.clear();
			ss << listens[j].second;
			IPPort = intToIp(listens[j].first) + ":" + ss.str();
			_IPPortToServer[IPPort].push_back(&_servers[i]);
		}
	}
}

// HELPER FUNCTION FOR CLUSTER::PARSE()
void	Cluster::parseConfig(std::vector<string> tokens)
{
	for (int i = 0; i < (int)tokens.size(); i++)
	{
		if (tokens[i++] == "server")
		{
			ServerBlock				server;
			if ((i = server.parseServer(tokens, i)) < 0)
				throw ClusterException(RED + _configPath + " error" RESET);
			this->_servers.push_back(server);
		}
		else
			throw ClusterException(RED + _configPath + " error: unknown directive: '" + tokens[--i] + "'" RESET);
	}
}

// ============================= INIT ALL SERVERS =============================

/**
 * @brief	Sets up all specified listening sockets.
 */
void	Cluster::init(void)
{
	int		listener;
	size_t	i = 0;
	string	IPPort;
	string	IP;
	string Port;
	size_t	colonPos;

	for (std::map<string, std::vector<ServerBlock *> >::iterator it = _IPPortToServer.begin(); it != _IPPortToServer.end(); it++)
	{
		IPPort = it->first;
		colonPos = IPPort.find(":");
		if (colonPos != string::npos)
		{
			IP = IPPort.substr(0, colonPos);
			Port = IPPort.substr(colonPos + 1);
			if ((listener = createListenerSocket(IP, Port)) == -1)
				throw ClusterException(RED "listener socket error" RESET);
			_pollFds[i].fd = listener;
			_pollFds[i++].events = POLLIN;
			_listenerToServer[listener] = it->second;
			_numOfFds++;
		}
	}
	if (_numOfFds == 0)
		throw ClusterException(RED "Could not set up cluster" RESET);
	LogStream::success() << "Webserv set up complete. Ready to accept connections." << std::endl;
}

// HELPER FUNCTION FOR CLUSTER::INIT()
int		Cluster::createListenerSocket(string IP, string Port)
{
	int	listener;
	int yes = 1;
	int	status;

	struct addrinfo hints = {}, *addr_list, *ptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// std::cout << "intended: " << IP << ":" << Port << "\n";
	if ((status = getaddrinfo(IP.c_str(), Port.c_str(), &hints, &addr_list)) != 0)
	{
		LogStream::error() << "getaddrinfo error: " << gai_strerror(status) << std::endl;
		return (-1);
	}
	for (ptr = addr_list; ptr != NULL; ptr = ptr->ai_next)
	{
		listener = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (listener < 0)
			continue ;
		if (fcntl(listener, F_SETFL, O_NONBLOCK) < 0 ||
			setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0 ||
			bind(listener, ptr->ai_addr, ptr->ai_addrlen) < 0)
		{
			LogStream::error() << "sth went wrong\n";
			close(listener);
			continue ;
		}
		break ;
	}
	if (ptr == NULL)
	{
		LogStream::error() << "socket, fcntl, setsockopt or bind error" << std::endl;
		freeaddrinfo(addr_list);
		return (-1);
	}
	struct sockaddr_in	actual_addr;
	socklen_t			addr_len = sizeof(actual_addr);
	getsockname(listener, (struct sockaddr *)&actual_addr, &addr_len);
	LogStream::info() << "Bound to port: " << ntohs(actual_addr.sin_port) << std::endl;
	freeaddrinfo(addr_list);
	if (listen(listener, 100) == -1)
	{
		LogStream::error() << "listen error" << std::endl;
		close(listener);
		return (-1);
	}
	return (listener);
}

void Cluster::removeFd(int i) {
	int fd = _pollFds[i].fd;

	close(fd);
	delete _clients[fd];
	_clients.erase(fd);

	if (i != _numOfFds - 1) {
		_pollFds[i] = _pollFds[_numOfFds - 1]; // Move last fd to current slot
	}
	_numOfFds--;


	LogStream::success() << "Closed connection [" << fd << "]" << std::endl;
}

// ============================== RUN ALL SERVERS =============================


void printFdEvents(const struct pollfd& pfd) {
	std::cout << "FD [" << pfd.fd << "] events: ";
	if (pfd.revents == 0) {
		std::cout << "None" << std::endl;
		return;
	}

	std::cout << pfd.revents << " ";
	if (pfd.revents & POLLIN)     std::cout << "POLLIN ";
	if (pfd.revents & POLLPRI)    std::cout << "POLLPRI ";
	if (pfd.revents & POLLOUT)    std::cout << "POLLOUT ";
	if (pfd.revents & POLLERR)    std::cout << "POLLERR ";
	if (pfd.revents & POLLHUP)    std::cout << "POLLHUP ";
	if (pfd.revents & POLLNVAL)   std::cout << "POLLNVAL ";
	if (pfd.revents & POLLRDHUP)  std::cout << "POLLRDHUP "; // If using epoll with Linux extension

	std::cout << std::endl;
}


ServerBlock *Cluster::resolveServer(HttpRequest &request) {
	ServerBlock *serverBlock = NULL;

	string hostHeader = request.headerGet("Host");
	if (hostHeader.empty()) {
		LogStream::error() << "Host header is missing in the request." << std::endl;
		serverBlock = &_servers[0];
	} 
	

	else {

		size_t colonPos = hostHeader.find(":");
		string hostName;
		string portNumber;

		request.setHost(portNumber);

		if (colonPos != std::string::npos) {
			hostName = hostHeader.substr(0, colonPos);
			portNumber = hostHeader.substr(colonPos + 1);
		} 
		
		else {
			hostName = hostHeader;
			portNumber = "";
		}

		

		std::vector<ServerBlock *> matchingServers;
		
		for (size_t serverIndex = 0; serverIndex < _servers.size(); serverIndex++) {
			std::vector<std::pair<uint32_t, int> > ports = _servers[serverIndex].getListen();

			for (size_t j = 0; j < ports.size(); j++) {
				if (portNumber == to_string(ports[j].second))
					matchingServers.push_back(&_servers[serverIndex]);
			}
		}

		serverBlock = matchingServers.empty() ? &_servers[0] : matchingServers[0];

		for (std::vector<ServerBlock *>::iterator it = matchingServers.begin(); it != matchingServers.end(); ++it) {
			const stringList& serverNames = (*it)->getServerName();
			
			if (serverNames.empty())
				continue;
				
			if (std::find(serverNames.begin(), serverNames.end(), hostName) != serverNames.end()) {
				LogStream::success() << "Matching server block found for Host: " << hostHeader << " to server [" << hostName << "]" << std::endl;
				serverBlock = *it;
				break;
			}
		}
		

		if (!serverBlock) {
			LogStream::error() << "No matching server block found for Host: " << hostHeader << std::endl;
			serverBlock = &_servers[0]; 
		}
	}
	LogStream::success() << "Resolved host " << hostHeader << " --> server " << (serverBlock - &_servers[0]) << std::endl;


	return serverBlock;
}


/**
 * @brief	Monitors each socket for incoming connections, request or reponses
 * 			(I/O operations).
 */
void Cluster::run(void) {
	int ready;
	int timeout = 500; // 500 ms timeout

	signal(SIGPIPE, SIG_IGN);


while (true) {

LogStream::log() << "listening on " << _numOfFds << " sockets | " << formatTime("%I:%M%p %S") << std::endl;
if ((ready = poll(_pollFds, _numOfFds, timeout)) == -1) {
	perror("poll");
	throw ClusterException("");
}

for (int i = 0; i < _numOfFds; i++) {
	if (_pollFds[i].revents & (POLLIN | POLLHUP)) {
		if (_listenerToServer.find(_pollFds[i].fd) != _listenerToServer.end()) {
			LogStream::pending() << "Handling new client " << _pollFds[i].fd << std::endl;
			handleNewClient(_pollFds[i].fd);
		} 
		else {
			char 		buffer[BUFFER_SIZE];
			string& 	requestBuffer 	= _clients[_pollFds[i].fd]->getRequestBuffer();
			HttpRequest &request  		= _clients[_pollFds[i].fd]->getRequest();

			LogStream::pending() << "Reading from [" << _pollFds[i].fd << "]" << std::endl;

			// -- READ FROM SOCKET --
			ssize_t byteRecv = recv(_pollFds[i].fd, buffer, BUFFER_SIZE - 1, 0);
			if (byteRecv <= 0) {
				if 		(byteRecv == 0)    LogStream::error() << "Client disconnected [" << _pollFds[i].fd << "]" << std::endl;
				else if (byteRecv == -1) { LogStream::error() << "No data available yet, will try again next iteration [" << _pollFds[i].fd << "]" << std::endl; break; } 
				else                       LogStream::error() << "Recv error" << std::endl;
				removeFd(i--);
				break;
			}
			requestBuffer.append(buffer, byteRecv);
				

			// -- EXTRACT CONTENT LENGTH --
			int contentLength = _clients[_pollFds[i].fd]->extractContentLength();
			if (contentLength == -1)
				continue;
			

			// -- CHECK IF FULL REQUEST RECEIVED --
			if (_clients[_pollFds[i].fd]->extractBodySize() < contentLength) {
				LogStream::pending() << "Waiting for more data from [" << _pollFds[i].fd << "]" << std::endl;
				continue;
			}
			
			LogStream::success() << "Full request received from [" << _pollFds[i].fd << "] with " << requestBuffer.size() << " bytes" << RESET << std::endl;
			

			// --- PARSE REQUEST ---
			_clients[_pollFds[i].fd]->handleRequest(requestBuffer);
			

			// --- SAVE REQUEST ---
			string outputFolder  = generateLogFileName(REQUESTS_FOLDER, request.getUid(), request.getMethod() + "_request_" + request.headerGet("path"));
			LogStream::log(outputFolder, std::ios::app) << request.getRawRequest() << std::endl;
			LogStream::success() << "Full request saved to " << outputFolder << std::endl;
			requestBuffer.clear();


			// --- PREPARE RESPONSE ---
			HttpResponse response    = HttpResponse(request, resolveServer(request));
			_clients[_pollFds[i].fd]->prepareForResponse(response);
			_pollFds[i].events = POLLOUT;
			break;
		}
	}

	if (_pollFds[i].revents & POLLOUT) {
		Client* client = this->_clients[_pollFds[i].fd];
		if (client->sendingResponse && client->getBytesLeft() > 0) {

			ssize_t sent = client->sendTo(_pollFds[i].fd);
			if (sent == -1) {
				LogStream::error() << "Send failed for client [" << _pollFds[i].fd << "], closing connection." << std::endl;
				removeFd(i--);
				continue;
			}
			
			if (client->getBytesLeft() != 0) 
				continue;

			LogStream::success(generateLogFileName("logs/responses/", client->getRequest().getUid(), client->getRequest().headerGet("path")), std::ios::app) << client->responseBuffer << std::endl;
			client->sendingResponse = false;
			LogStream::success() << "Message sent successfully." << std::endl;
			if (client->isKeepAlive()) {
				LogStream::pending() << "Keeping connection alive for client [" << _pollFds[i].fd << "]" << std::endl;
				_pollFds[i].events = POLLIN;
			} 
			else {
				LogStream::pending() << "Closing connection for client [" << _pollFds[i].fd << "]" << std::endl;
				removeFd(i--);
			}
		}
	}
}
}
}

// HELPER FUNCTION FOR CLUSTER::RUN()
void	Cluster::handleNewClient(int listenerFd)
{
	int newClient;

	if ((newClient = accept(listenerFd, NULL, NULL)) < 0)
		perror("accept");
	if (fcntl(newClient, F_SETFL, O_NONBLOCK) < 0)
		perror("fcntl");
	else
	{
		this->_clients[newClient] = new Client();
		_pollFds[_numOfFds].fd = newClient;
		_pollFds[_numOfFds].events = POLLIN; // Explicitly set to POLLIN
		LogStream::success() << "New client connection received: [" << _pollFds[_numOfFds].fd << "]" << std::endl; 
		_pollFds[_numOfFds++].events = POLLIN;
	}	
}

// ================================= UTILS ====================================

/**
 * @brief	Trims leading and trailing characters as specified by delims.
 */
string	strTrim(string str, string delims)
{
	size_t start = str.find_first_not_of(delims);
	if (start != string::npos)
		str = str.substr(start);
	else
		str = "";
    size_t end = str.find_last_not_of(delims);
	if (end != string::npos)
		str = str.substr(0, end + 1);
	else
		str = "";
	return (str);
}
