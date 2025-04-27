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

HttpRequest::HttpRequest(HttpMethod method, string path, string version) {
    // Constructor implementation
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
    infoStream << "\n-- request --" << std::endl;
    // infoStream << GREEN << "\tpath: " << RESET         << this->headerGet("path") << std::endl;
    // infoStream << GREEN << "\tpath_info: " << RESET    << this->headerGet("path_info") << std::endl;
    // infoStream << GREEN << "\tquery_string: " << RESET << this->headerGet("query_string") << std::endl;

    stringDict::iterator it = this->headerParameters.begin();
    stringDict::iterator end = this->headerParameters.end();

    while(it != end) {
        infoStream << "" << it->first << ": " << it->second << std::endl;
        ++it;
    }

    infoStream << "method: " << this->getMethod() << std::endl;

    return infoStream.str();
}
