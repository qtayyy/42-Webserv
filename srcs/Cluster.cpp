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

    request.headerSet("path", path);
    request.headerSet("path_info", path_info);
    request.headerSet("query_string", "name=John&age=30");
    request.headerSet("method", "GET");

    return request;
}

HttpRequest mockUploadPOSTRequest(string path, string path_info) {
    HttpRequest request;

	request.setRawRequest(readFileContent("test_post_request"));
	request.setBody(readFileContent("test_post_request"));

    request.headerSet("path", path);
    request.headerSet("path_info", path_info);
    request.headerSet("query_string", "name=John&age=30");
    request.headerSet("method", "POST");
    request.headerSet("content_type", "multipart/form-data; boundary=boundary");

    return request;
}

HttpRequest mockUploadGETRequest() {
    HttpRequest request;

    request.headerSet("path", "/upload/upload.py");
    request.headerSet("path_info", "webserv.pdf");
    request.headerSet("query_string", "name=John&age=30");
    request.headerSet("method", "GET");

    return request;
}

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::string constructHttpResponse(std::string pdf_file) {
	std::ifstream file(pdf_file.c_str(), std::ios::in | std::ios::binary);

    // Read the entire file content into a string
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Send HTTP response headers
    std::string headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: application/pdf\r\n";
    headers += "Connection: close\r\n";
    headers += "Content-Length: " + to_string(file_content.size()) + "\r\n";
    headers += "\r\n";
    
    // Combine headers and file content
    std::string response = headers + file_content;
    
    // Send the entire response in a single send call

    std::cout << "" << response.c_str() << std::endl;

    return response;
}

#define BUFF_SIZE 30000

void sendHttpResponse(int client_sock, const std::string &pdf_file) {
	std::ifstream file(pdf_file.c_str(), std::ios::binary | std::ios::ate);
	if (!file) {
		std::string not_found = "HTTP/1.1 404 Not Found\r\n"
								"Content-Type: text/plain\r\n"
								"Content-Length: 13\r\n"
								"Connection: close\r\n\r\n"
								"404 Not Found";
		send(client_sock, not_found.c_str(), not_found.size(), 0);
		return;
	}

	std::ifstream::pos_type file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the entire file content into a string
	std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	// Send HTTP response headers
	std::string headers = "HTTP/1.1 200 OK\r\n"
						  "Content-Type: application/pdf\r\n"
						  "Content-Length: " + to_string(file_content.size()) + "\r\n"
						  "\r\n\r\n";

	// Combine headers and file content
	std::string response = headers + file_content;

	// Send the entire response in a single send call
	int bytes = send(client_sock, response.c_str(), response.size(), 0);
	std::cout << "response:: " << response << std::endl;
	std::cout << "bytes:: " << bytes << std::endl;
	std::ofstream outFile("output.log");
	if (outFile.is_open()) {
		outFile << response;
		outFile.close();
	} 
	else
		std::cerr << "Unable to open file for writing" << std::endl;
}

// ============================== RUN ALL SERVERS =============================

/**
 * @brief	Monitors each socket for incoming connections, request or reponses
 * 			(I/O operations).
 */
void Cluster::run(void) {
    int ready;
    int timeout = 500; // 500 ms timeout

    while (true) {
        if ((ready = poll(_pollFds, _numOfFds, timeout)) == -1) {
            perror("poll");
            throw ClusterException("");
        }
        for (int i = 0; i < _numOfFds; i++) {
            if (_pollFds[i].revents & (POLLIN | POLLHUP)) { // If an fd is ready for reading
                if (_listenerToServer.find(_pollFds[i].fd) != _listenerToServer.end())
                    handleNewClient(_pollFds[i].fd);
                else { // Note: if client disconnects, have to close its fd and remove from the poll struct
                    this->_clients[_pollFds[i].fd]->handleRequest(); // Ethan's part
                    _pollFds[i].events = POLLOUT;
                }
            }
            if (_pollFds[i].revents & POLLOUT) { // If an fd is ready for writing
				//sendHttpResponse(_pollFds[i].fd, "/home/cooper/coreProgram/qi_ter_webserv/public/upload/webserv.pdf");
				

				
				
				HttpRequest request = mockUploadGETRequest();
				HttpResponse response = HttpResponse(request, &_servers[0]);
				
				string finalMsg = response.getFinalResponseMsg();
				
				// finalMsg = readFileContent("output2.log");


				int bytes = send(_pollFds[i].fd, finalMsg.c_str(), finalMsg.size(), 0);

				// close(_pollFds[i].fd);
				_clients.erase(_pollFds[i].fd);
				_pollFds[i] = _pollFds[--_numOfFds];
				_pollFds[i].events = POLLIN;
				i--;

				
				
				//sendHttpResponse(_pollFds[i].fd, "/home/cooper/coreProgram/qi_ter_webserv/public/upload/webserv.pdf");

				// close(_pollFds[i].fd);
				// _clients.erase(_pollFds[i].fd);
				// _pollFds[i] = _pollFds[--_numOfFds];
				// _pollFds[i].events = POLLIN;
				// i--;
            }
        }
    }
}
// HttpRequest request = mockUploadPOSTRequest("/upload/", "/dir2");
//HttpRequest request = mockRequest("/upload.html", "/dir2");
// HttpRequest request = mockUploadGETRequest();

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
