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
	std::vector<std::string>	tokens;
	try
	{
		tokens = tokenizeConfig(this->_configPath);
		parseConfig(tokens);
		mapIPPortToServer();
		std::cout << GREEN "Config file parsing complete.\n" RESET;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

// HELPER FUNCTION FOR CLUSTER::PARSE()
std::vector<std::string>	Cluster::tokenizeConfig(std::string &configPath)
{
	std::ifstream configFile(configPath.c_str());

	if (!configFile.is_open())
		throw ClusterException(RED + configPath + ": No such file or directory" RESET);
	std::string	line;
	std::vector<std::string>	tokens;
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
			if ((end = line.find_first_of(" \t;", start)) == std::string::npos)
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
	std::string	IPPort;
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
void	Cluster::parseConfig(std::vector<std::string> tokens)
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
	std::string	IPPort;
	std::string	IP;
	std::string Port;
	size_t	colonPos;

	for (std::map<std::string, std::vector<ServerBlock *> >::iterator it = _IPPortToServer.begin(); it != _IPPortToServer.end(); it++)
	{
		IPPort = it->first;
		colonPos = IPPort.find(":");
		if (colonPos != std::string::npos)
		{
			IP = IPPort.substr(0, colonPos);
			Port = IPPort.substr(colonPos + 1);
			if ((listener = createListenerSocket(IP, Port)) == -1)
				throw ClusterException(RED "listener socket error" RESET);
			if (_pollFds != NULL)
			{
				_pollFds[i].fd = listener;
				_pollFds[i++].events = POLLIN;
				_listenerToServer[listener] = it->second;
				_numOfFds++;
			}
		}
	}
	if (_numOfFds == 0)
		throw ClusterException(RED "Could not set up cluster" RESET);
	std::cout << GREEN "Webserv set up complete. Ready to accept connections.\n" RESET;
}

// HELPER FUNCTION FOR CLUSTER::INIT()
int		Cluster::createListenerSocket(std::string IP, std::string Port)
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
		std::cerr << RED "getaddrinfo error: " << gai_strerror(status) << RESET << std::endl;
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
			std::cerr << RED "sth went wrong\n" RESET;
			close(listener);
			continue ;
		}
		break ;
	}
	if (ptr == NULL)
	{
		std::cerr << RED "socket, fcntl, setsockopt or bind error" RESET<< std::endl;
		return (-1);
	}
	struct sockaddr_in	actual_addr;
	socklen_t	addr_len = sizeof(actual_addr);
	getsockname(listener, (struct sockaddr *)&actual_addr, &addr_len);
	std::cout << YELLOW "Bound to port: " << ntohs(actual_addr.sin_port) << std::endl << RESET;
	freeaddrinfo(addr_list);
	if (listen(listener, 100) == -1)
	{
		std::cerr << RED "listen error" RESET << std::endl;
		close(listener);
		return (-1);
	}
	return (listener);
}

HttpRequest mockRequest(string path, string path_info) {
    HttpRequest request;

    request.setParam("path", path);
    request.setParam("path_info", path_info);
    request.setParam("query_string", "name=John&age=30");
    request.setParam("method", "GET");

    return request;
}

// ============================== RUN ALL SERVERS =============================

/**
 * @brief	Monitors each socket for incoming connections, request or reponses
 * 			(I/O operations).
 */
void	Cluster::run(void)
{
	int	ready;
	int	timeout = 500; // 500 ms timeout

	while (true)
	{
		if ((ready = poll(_pollFds, _numOfFds, timeout)) == -1)
		{
			perror("poll");
			throw ClusterException("");
		}
		for (int i = 0; i < _numOfFds; i++)
		{
			if (_pollFds[i].revents & (POLLIN | POLLHUP)) // If an fd is ready for reading
			{
				if (_listenerToServer.find(_pollFds[i].fd) != _listenerToServer.end())
					handleNewClient(_pollFds[i].fd);
				else // Note: if client disconnects, have to close its fd and remove from the poll struct
				{
					this->_clients[_pollFds[i].fd]->handleRequest(); // Ethan's part

					HttpRequest request = mockRequest("test.csv", "test.csv");

					HttpResponse response = HttpResponse::createHttpResponse(request);
					send(_pollFds[i].fd, response.getFinalResponseMsg().c_str(), response.getFinalResponseMsg().size(), 0);
					close(_pollFds[i].fd);
					_clients.erase(_pollFds[i].fd);
					_pollFds[i] = _pollFds[--_numOfFds]; // Remove the fd from the poll array
					i--; // Adjust the index to check the swapped fd
				}
			}
			if (_pollFds[i].revents & POLLOUT) // // If an fd is ready for writing
			{
				this->_clients[_pollFds[i].fd]->handleResponse(); // Sheldon's part
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
		this->_clients[newClient] = new Client(); // Ethan's part (prob need to pass info about config - ServerBlock)
		_pollFds[_numOfFds].fd = newClient;
		_pollFds[_numOfFds++].events = POLLIN;
		std::cout << GREEN "New client connection received\n" RESET; 
	}	
}

// ================================= UTILS ====================================

/**
 * @brief	Trims leading and trailing characters as specified by delims.
 */
std::string	strTrim(std::string str, std::string delims)
{
	size_t start = str.find_first_not_of(delims);
	if (start != std::string::npos)
		str = str.substr(start);
	else
		str = "";
    size_t end = str.find_last_not_of(delims);
	if (end != std::string::npos)
		str = str.substr(0, end + 1);
	else
		str = "";
	return (str);
}
