#include "HttpRequest.hpp"

void HttpRequest::appendFormBlock(stringDict formBlock) {
    formBlocks.push_back(formBlock);
}

stringDict *HttpRequest::getFormBlock(int index) {
    return &formBlocks[index];
}

void HttpRequest::setBody(string body) {
    this->body = body;
}

string &HttpRequest::getBody() {
    return body;
}

void HttpRequest::headerSet(string key, string value) {
    headerParameters[key] = value;
    // std::cout << "SETTING " << key << "to " << headerParameters[key] << std::endl;
}

void HttpRequest::setMethod(string method) {
    this->method = method;
}

string HttpRequest::getMethod() {
    return method;
}

string HttpRequest::headerGet(string key) {
    return headerParameters[key];
}

HttpRequest::HttpRequest() {
    // Constructor implementation
}

HttpRequest::~HttpRequest() {
    // Destructor implementation
}

void HttpRequest::setRawRequest(string rawRequest) {
    this->rawRequest = rawRequest;
}

string HttpRequest::getRawRequest() const {
    return rawRequest;
}


string HttpRequest::preview() { 
    std::ostringstream infoStream;
    infoStream << "-- request --" << std::endl;

    infoStream << "method: " << this->getMethod() << std::endl;

    infoStream << "--------------------------|" << std::endl;
    stringDict::iterator it = this->headerParameters.begin();
    stringDict::iterator end = this->headerParameters.end();

    // Calculate the maximum key length for alignment
    size_t maxKeyLength = 0;
    for (it = headerParameters.begin(); it != end; ++it) {
        if (it->first.length() > maxKeyLength) {
            maxKeyLength = it->first.length();
        }
    }

    for (it = headerParameters.begin(); it != end; ++it) {
        infoStream << std::setw(maxKeyLength) << std::left << it->first << " | " << it->second << std::endl;
    }

    infoStream << "__________________________|" << std::endl;
    infoStream << "body size (bytes): " << this->body.size() << std::endl;
    infoStream << "raw request size (bytes): " << this->rawRequest.size() << std::endl;

    return infoStream.str();
}
