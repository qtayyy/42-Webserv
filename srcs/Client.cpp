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

void Client::receiveRequest(ssize_t read_buf, char *buffer) {
    //Reads specified number of bytes from the socket
    if (read_buf <= 0) {
        if (read_buf == 0) {
            std::cout << "hang" << read_buf << std::endl;
        }
        
        else {
            std::cerr << "Error receiving data on socket fd " << socket_fd << ": " << strerror(errno) << std::endl;
            perror("RECV");
        }
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
    // std::cout << "PATH" << path << std::endl;
    // std::cout << "Query string" << queryString << std::endl;
    // std::cout << "abs path" << path << std::endl;

    request.preview();
}

void Client::parseHeaders(const std::string& headers) {
    std::string headersCopy = headers; 
    size_t pos = 0;
    while ((pos = headersCopy.find("\r\n")) != std::string::npos) {
        std::string headerLine = headersCopy.substr(0, pos);
        headersCopy = headersCopy.substr(pos + 2);
        size_t colonPos = headerLine.find(": ");
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 2);
            request.headerSet(key, value);
        }
    }
}

//parses the chunked body
void Client::parseChunkedBody() {
    size_t headerEnd;
    std::string fullBody;
    headerEnd = request_buf.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        std::cerr << "Invalid chunked request format" << std::endl;
        return;
    }
    size_t pos;
	size_t chunkSize;
    pos = headerEnd + 4;
    chunkSize = 1;

    size_t lineEnd;
    while (chunkSize != 0) {
        lineEnd = request_buf.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            std::cerr << "Invalid chunk format" << std::endl;
            return;
        }
		std::string hexSize;
        hexSize = request_buf.substr(pos, lineEnd - pos);
        chunkSize = parseChunkSize(hexSize);
        pos = lineEnd + 2;
        if (pos + chunkSize > request_buf.length()) {
            std::cerr << "Read over chunk data length" << std::endl;
            return;
        }
        if (chunkSize > 0) {
            fullBody += request_buf.substr(pos, chunkSize);
            pos += chunkSize + 2;
        }
    }
    request.setBody(fullBody);
}

size_t Client::parseChunkSize(const std::string& hexChunk) {
    // Remove any chunk extensions
    size_t semicolonPos = hexChunk.find(';');
    std::string cleanHex = hexChunk;
    if (semicolonPos != std::string::npos) {
        cleanHex = hexChunk.substr(0, semicolonPos);
    }
    return hexToDec(cleanHex);
}

size_t Client::hexToDec(const std::string& hex) {
    size_t result = 0;
    for (size_t i = 0; i < hex.length(); i++) {
        result *= 16;
        if (hex[i] >= '0' && hex[i] <= '9') {
            result += hex[i] - '0';
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            result += hex[i] - 'a' + 10;
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            result += hex[i] - 'A' + 10;
        } else {
            std::cerr << "Invalid hex character" << std::endl;
            return 0;
        }
    }
    return result;
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
	
	// Check if there is chunked transfer encoding
	if (request.headerGet("Transfer-Encoding") == "chunked") {
		parseChunkedBody(); 
	}
	else {
		parseBody(headerEnd);
	}
    request.setRawRequest(request_buf);
}

void Client::handleRequest(ssize_t byteRec, char *buffer) {
    receiveRequest(byteRec, buffer);
    parseRequest();
    request.preview();
}

HttpRequest& Client::getRequest() {
    return request;
}

