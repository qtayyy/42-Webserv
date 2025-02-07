#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../includes/Webserv.hpp"
#include "HttpRequest.hpp"
#include "StatusHandler.hpp"
#include "Utils.hpp"
#include "HttpException.hpp"
#include "CGI.hpp"
#include "ServerBlock.hpp"

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

    ServerBlock *_serverBlockRef;
    LocationBlock *_locationBlockRef;
    static bool containsIndexFile(string path);

std::vector<LocationBlock>::iterator getRelevantLocationBlock(ServerBlock *serverBlock, string path);

public:
    HttpResponse(string content, string contentType, int statusCode);
    HttpResponse(HttpRequest &request, ServerBlock &ServerBlock);

    string getRawContent() const { return rawContent; }
    int getContentLength() const { return contentLength; }
    int getStatusCode() const { return statusCode; }
    string getMessage() const { return message; }
    string getContentType() const { return contentType; }
    string getFinalResponseMsg() const { return finalResponseMsg; }

    static string decideCGIToUse(string resourcePath);
    static string getContentType(const string& resourcePath);
    string reroutePath(const string &urlPath);
    static string createHttpResponseString(
        const string &fileContent,
        const string &resourceType,
        const string &statusCode,
        const string &statusMessage);
    void setHttpResponseSelf(string content, string resourceType, int statusCode);
    static HttpResponse createHttpErrorResponse(int statusCode);
    static string generateAutoIndexHtml(string path);
    static HttpResponse createHttpResponse(HttpRequest &request);

    void callCGIResponse(string cgiPath, string fileToHandle, HttpRequest request);
    HttpResponse(HttpRequest &request, ServerBlock *ServerBlock);
    void initErrorHttpResponse(int statusCode);
};

#endif // HTTP_RESPONSE_HPP