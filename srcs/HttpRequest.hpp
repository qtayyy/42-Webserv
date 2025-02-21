#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../includes/Webserv.hpp"

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
private:
    string method;
    string     body;
    stringDict headerParameters;
    std::vector<stringDict> formBlocks;
    string rawRequest;

public:
    void        appendFormBlock(stringDict formBlock);
    stringDict *getFormBlock(int index);
    void        setBody(string body);
    string      getBody();


    void setMethod(string method);
    string getMethod();

    void        headerSet(string key, string value);
    string headerGet(string key);

    void   setRawRequest(string rawRequest);
    string getRawRequest() const;

    HttpRequest(HttpMethod method, string path, string version);
    HttpRequest();
    ~HttpRequest();

    string getMethod() const;
    void printInfo();
};

#endif