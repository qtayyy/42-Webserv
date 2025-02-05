#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../includes/Webserv.hpp"
#include "HttpRequest.hpp"
#include "StatusHandler.hpp"
#include "Utils.hpp"
#include "HttpException.hpp"
#include "CGI.hpp"

typedef std::map<string, string> stringDict;

class HttpResponse {
private:
    static const stringDict contentTypeMap;
    static stringDict createContentTypeMap();
    string  rawContent;
    int     contentLength;
    int     statusCode;
    string  message;
    string  contentType;
    string  finalResponseMsg;

public:
    HttpResponse(string content, string contentType, int statusCode);

    string getRawContent() const { return rawContent; }
    int getContentLength() const { return contentLength; }
    int getStatusCode() const { return statusCode; }
    string getMessage() const { return message; }
    string getContentType() const { return contentType; }
    string getFinalResponseMsg() const { return finalResponseMsg; }

    static string decideCGIToUse(string resourcePath);
    static string getContentType(const string& resourcePath);
    static string createHttpResponseString(
        const string& fileContent, 
        const string& resourceType, 
        const string& statusCode, 
        const string& statusMessage);
    static HttpResponse createHttpErrorResponse(int statusCode);
    static string generateAutoIndexHtml(string path);
    static HttpResponse createHttpResponse(HttpRequest &request);
};

#endif // HTTP_RESPONSE_HPP