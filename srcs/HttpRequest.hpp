#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../includes/Webserv.hpp"

class HttpRequest {
private:
    string     method;
    string     body;
    stringDict headerParameters;
    string     rawRequest;
    string     raw;
    string     queryString;
    string     absolutePath;
    string     _path;

public:
    
    /* CONSTRUCTOR/DESTRUCTOR */
    
    HttpRequest();
    ~HttpRequest();

    void        appendFormBlock(stringDict formBlock);
    stringDict *getFormBlock(int index);
    void        setBody(string body);
    string      &getBody();

    /* GETTER/SETTERS */

    void   setMethod(string method);
    string getMethod() const;
    
    void   setQueryString(string queryString);
    string getQueryString() const;
    
    void   setPath(string path);
    string getPath() const;

    void   headerSet(string key, string value);
    string headerGet(string key);

    void   setRawRequest(string rawRequest);
    string getRawRequest() const;

    string preview();
    
};

#endif