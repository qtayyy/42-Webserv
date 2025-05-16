#include "HttpRequest.hpp"

/* CONSTRUCTORS/DESTRUCTORS */

HttpRequest::HttpRequest() : _uid(generateRandomID(10)) { }
HttpRequest::~HttpRequest() { }


string const &HttpRequest::getUid() const {
    return _uid;
}

void HttpRequest::setHost(string hostNumber) {
    this->hostNumber = hostNumber;
}

string HttpRequest::getHost() {
    return this->hostNumber;
}

/* GETTERS/SETTERS */

void   HttpRequest::setBody(string body)                { this->body = body; }
string &HttpRequest::getBody()                          { return body; }

string HttpRequest::headerGet(string key)               { return headerParameters[key]; }
void   HttpRequest::headerSet(string key, string value) { headerParameters[key] = value; }

string HttpRequest::getMethod() const                   { return method; }
void   HttpRequest::setMethod(string method)            { this->method = method; }

string HttpRequest::getRawRequest() const               { return rawRequest; }
void   HttpRequest::setRawRequest(string rawRequest)    { this->rawRequest = rawRequest; }

void   HttpRequest::setQueryString(string queryString)  { this->queryString = queryString; }
string HttpRequest::getQueryString() const              { return queryString; }

void   HttpRequest::setPath(string path)                { this->_path = path; }
string HttpRequest::getPath() const                     { return _path; }



string HttpRequest::preview() { 
    std::ostringstream infoStream;
    infoStream << "-- request --" << std::endl;

    infoStream << "method: " << this->getMethod() << std::endl;

    stringDict::iterator it = this->headerParameters.begin();
    stringDict::iterator end = this->headerParameters.end();

    size_t maxKeyLength = 0;
    for (it = headerParameters.begin(); it != end; ++it) {
        if (it->first.length() > maxKeyLength)
            maxKeyLength = it->first.length();
    }

    std::string separatorLine(maxKeyLength + 1, '_'); // +3 for " |"
    infoStream << separatorLine << "." << std::endl;

    for (it = headerParameters.begin(); it != end; ++it)
        infoStream << std::setw(maxKeyLength) << std::left << it->first << " | " << it->second << std::endl;

    separatorLine = string(maxKeyLength + 1, '_'); // +3 for " |"
    infoStream << separatorLine << "|" << std::endl;
    infoStream << "body size (bytes): "         << this->body.size() << std::endl;
    infoStream << "raw request size (bytes): "  << this->rawRequest.size() << std::endl;

    return infoStream.str();
}
