/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ethanlim <ethanlim@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 02:31:50 by ethanlim          #+#    #+#             */
/*   Updated: 2025/03/13 11:33:47 by ethanlim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd) : socket_fd(fd), request_buf("") {
}

Client::~Client() {
}

void Client::receiveRequest() {
    char buffer[BUFFER_SIZE];
    ssize_t read_buf;
    
    //Reads specified number of bytes from the socket
    read_buf = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
    if (read_buf <= 0) {
        std::cerr << "Couldn't read from socket" << std::endl;
        return;
    }
    
    buffer[read_buf] = '\0';
    request_buf = buffer;
}

//parses the first line into the 3 parts(methods, path, query)
//this first part is to get the method(GET, POST)
void Client::parseRequestLine(const std::string& requestLine) {
    size_t methodEnd = requestLine.find(" ");
    if (methodEnd == std::string::npos) {
        std::cerr << "Invalid request line" << std::endl;
        return;
    }
    
    std::string method = requestLine.substr(0, methodEnd);
    request.setMethod(method);
    // this second part is to get the path (with or without ? [query string])
    size_t pathStart = methodEnd + 1;
    size_t pathEnd = requestLine.find(" ", pathStart);
    if (pathEnd == std::string::npos) {
        std::cerr << "Invalid request line" << std::endl;
        return;
    }
	//stores full path whether it be with query string or not 
    std::string fullPath = requestLine.substr(pathStart, pathEnd - pathStart);
    
    // split path and query string if there is 
    size_t queryStart = fullPath.find("?");
    std::string path = fullPath;
    std::string queryString = "";
    if (queryStart != std::string::npos) {
        path = fullPath.substr(0, queryStart);
        queryString = fullPath.substr(queryStart + 1);
    }

    // Store path information in key value pair(map)
    request.headerSet("path", path);
    request.headerSet("queryString", queryString);
    request.headerSet("absolute_path", path);
}

void Client::parseHeaders(const std::string& headers) {
    size_t pos = 0;
    while ((pos = headers.find("\r\n")) != std::string::npos) {
        std::string headerLine = headers.substr(0, pos);
        headers = headers.substr(pos + 2);

        size_t colonPos = headerLine.find(": ");
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 2);
            request.headerSet(key, value);
        }
    }
}

//parses the body and stores it
void Client::parseBody(size_t headerEnd) {
    if (headerEnd + 4 < request_buf.length()) {
        std::string body = request_buf.substr(headerEnd + 4);
        request.setBody(body);
    }
}

void Client::parseRequest() {
    // Find end of first line
    size_t firstLineEnd = request_buf.find("\r\n");
    if (firstLineEnd == std::string::npos) {
        std::cerr << "Invalid request format" << std::endl;
        return;
    }

    // Parse request line
    std::string requestLine = request_buf.substr(0, firstLineEnd);
    parseRequestLine(requestLine);

    // Parse headers
    size_t headerStart = firstLineEnd + 2;
    size_t headerEnd = request_buf.find("\r\n\r\n", headerStart);
    if (headerEnd == std::string::npos) {
        std::cerr << "No headers found" << std::endl;
        return;
    }
    std::string headers = request_buf.substr(headerStart, headerEnd - headerStart);
    parseHeaders(headers);
    parseBody(headerEnd);
    request.setRawRequest(request_buf);
}

void Client::handleRequest() {
    receiveRequest();
    parseRequest();
}

HttpRequest& Client::getRequest() {
    return request;
}

