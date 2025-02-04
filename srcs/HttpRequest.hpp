#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Header.hpp"

enum HttpMethod 
{
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    NONE  
};



class HttpRequest {
    HttpMethod method;
    string path;
    string version;
    string query;
    string urlFragment;
    
    

    stringDict requestParameters;

    public:
        void setParam(string key, string value) {
            requestParameters[key] = value;
        }

        string getParam(string key) {
            return requestParameters[key];
        }

        stringDict getRequestParameters() {
            return requestParameters;
        }

        void setRequestParameter(stringDict requestParameters) {
            this->requestParameters = requestParameters;
        }

        HttpRequest(HttpMethod method, string path, string version) : method(GET), path(path), version(version) {}
        HttpRequest() : method(GET), path(""), version("") {}
        ~HttpRequest() {}
        HttpRequest(const HttpRequest &other) {
            method = other.method;
            path = other.path;
            version = other.version;
        }

        HttpRequest &operator=(const HttpRequest &other) {
            if (this != &other) {
                method = other.method;
                path = other.path;
                version = other.version;
            }
            return *this;
        }


        string getMethod() const {
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


        string getPath() const{
            return path;
        }


        string getVersion() const {
            return version;
        }
        
        void printInfo() { 
            std::cout << "\n-- request --" << std::endl;
            std::cout << GREEN << "\tpath: " << RESET         << this->getParam("path") << std::endl;
            std::cout << GREEN << "\tpath_info: " << RESET    << this->getParam("path_info") << std::endl;
            std::cout << GREEN << "\tquery_string: " << RESET << this->getParam("query_string") << std::endl;
            std::cout << GREEN << "\tmethod: " << RESET       << this->getParam("method") << std::endl;
        }
};

#endif