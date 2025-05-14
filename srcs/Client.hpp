#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "HttpRequest.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "Log.hpp"
#include "Utils.hpp"

class Client
{
	public:
		Client(int fd);
		~Client();

        std::string& getRecvBuffer(); // Get the buffer for accumulating data
        bool isRequestReady() const; // Check if the request is fully parsed
        void reset(); // Reset the state for keep-alive connections

		void		parseRequest();
        void handleRequest(const std::string &buffer);
        HttpRequest &getRequest();
		std::string &getRequestBuffer() { return request_buf; }

	private:
		void		parseRequestLine(const std::string& request_line);
		void		parseHeaders(const std::string& headers);
		void		parseBody(size_t header_end);
		void		parseChunkedBody();
		size_t		parseChunkSize(const std::string& chunk_size);
		size_t		hexToDec(const std::string& hex);
		
		int			socket_fd;
		std::string request_buf; 
		bool		headers_parsed;
		bool		is_chunked; 
		size_t		content_length; 
		bool		request_ready; 
		HttpRequest request;
};

#endif
