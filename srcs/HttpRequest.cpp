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

string HttpRequest::getBody() {
    return body;
}

void HttpRequest::headerSet(string key, string value) {
    headerParameters[key] = value;
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


void HttpRequest::printInfo() { 
    std::cout << "\n-- request --" << std::endl;
    std::cout << GREEN << "\tpath: " << RESET         << this->headerGet("path") << std::endl;
    std::cout << GREEN << "\tpath_info: " << RESET    << this->headerGet("path_info") << std::endl;
    std::cout << GREEN << "\tquery_string: " << RESET << this->headerGet("query_string") << std::endl;
    std::cout << GREEN << "\tmethod: " << RESET       << this->getMethod() << std::endl;
}
