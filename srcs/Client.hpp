#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "HttpRequest.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>


class Client
{
	public:
		Client(int fd);
		~Client();

		void		receiveRequest(ssize_t read_buf, char *buffer);
		void		parseRequest();
		void		handleRequest(ssize_t byteRec, char *buffer);
		HttpRequest &getRequest();
	
	private:
		void		parseRequestLine(const std::string& request_line);
		void		parseHeaders(const std::string& headers);
		void		parseBody(size_t header_end);
		void		parseChunkedBody();
		size_t		parseChunkSize(const std::string& chunk_size);
		size_t		hexToDec(const std::string& hex);
		
		int			socket_fd;
		std::string	request_buf;
		HttpRequest request;

		
};

#endif
