#include "HttpRequest.hpp"

void HttpRequest::appendFormBlock(HttpRequestFormBlock formBlock) {
    formBlocks.push_back(formBlock);
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

string HttpRequest::getMethod() const {
    switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case PUT:
            return "PUT";
        case DELETE:
            return "DELETE";
        case HEAD:
            return "HEAD";
        default:
            return "NONE";
    }
}

void HttpRequest::printInfo() { 
    std::cout << "\n-- request --" << std::endl;
    std::cout << GREEN << "\tpath: " << RESET         << this->headerGet("path") << std::endl;
    std::cout << GREEN << "\tpath_info: " << RESET    << this->headerGet("path_info") << std::endl;
    std::cout << GREEN << "\tquery_string: " << RESET << this->headerGet("query_string") << std::endl;
    std::cout << GREEN << "\tmethod: " << RESET       << this->headerGet("method") << std::endl;
}