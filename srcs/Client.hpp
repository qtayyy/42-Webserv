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

		void		receiveRequest();
		void		parseRequest();
		void		handleRequest();
		HttpRequest &getRequest();
	private:
		int			socket_fd;
		std::string		request_buf;
		HttpRequest request;

		
};

#endif
