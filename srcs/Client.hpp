#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "HttpRequest.hpp"

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "Log.hpp"
#include "Utils.hpp"
#include "HttpResponse.hpp"

class Client
{
	public:

		bool 		sendingResponse;
		string 		responseBuffer;

		// -- CONSTRUCTORS/DESTRUCTORS --
		Client();
		~Client();

		
        // -- GETTERS/EXTRACTORS --
		ssize_t 	getBytesLeft();
        int 		extractContentLength();
        int     	extractBodySize();
        bool 		isKeepAlive();
        HttpRequest &getRequest();
		string 		&getRequestBuffer() { return _requestBuf; }
        
		// -- SETTERS --
        ssize_t 	sendTo(int fd);
		
		// -- REQUEST HANDLING --
        void		parseRequest();
        void		handleRequest(const string &buffer);
		
		// -- RESPONSE HANDLING --
        void		prepareForResponse(HttpResponse &response);
		
	private:

		// -- PARSING FUNCTIONS --
		void		parseRequestLine(const string& request_line);
		void		parseHeaders(const string& headers);
		void		parseBody(size_t header_end);
		void		parseChunkedBody();
		size_t		parseChunkSize(const string& chunk_size);
		size_t		hexToDec(const string& hex);
		
		string		_requestBuf; 
		bool 		_keepConnectionAlive;
		HttpRequest _request;
		ssize_t 	_bytesSent;
    	ssize_t 	_bytesLeft;


};

#endif
