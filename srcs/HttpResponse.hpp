#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../includes/Webserv.hpp"
#include "HttpRequest.hpp"
#include "StatusHandler.hpp"
#include "Utils.hpp"
#include "HttpException.hpp"
#include <ctime>
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
    string containsIndexFile(string path);

    LocationBlock *getRelevantLocationBlock(const string &path, ServerBlock *serverBlock);

    void initRedirectResponse(string &redirectUrl, int statusCode);

    string applyAlias(string &path);

    std::vector<LocationBlock>::iterator getRelevantLocationBlock(ServerBlock *serverBlock, string path);

    int httpStatusCode;       // HTTP status code (e.g., 404, 500)
    string errorMessage;      // Short error message
    string details;           // Detailed explanation
    string requestUrl;        // URL that caused the error
    string method;            // HTTP method (GET, POST, etc.)
    std::time_t timestamp;    // Error occurrence time

    std::pair<std::string, std::string> CodeToMessage(int code) const;
    string createErrorPage(string errorPagePath, int statusCode) const;
    void displayError() const;

public:
    HttpResponse(string content, string contentType, int statusCode);
    HttpResponse(HttpRequest &request, ServerBlock &ServerBlock);

    string getRawContent() const;
    int    getContentLength() const;
    int    getStatusCode() const;
    string getMessage() const;
    string getContentType() const;
    string getFinalResponseMsg() const;
    string getErrorMsg() const;
    string getDetails() const;
    string getRequestUrl() const;
    string getMethod() const;
    string getTimestamp() const;

    bool isCGI(const string &resourcePath);

    string decideCGIToUse(string resourcePath);
    static string getContentType(const string& resourcePath);
    string reroutePath(string urlPath);
    static string createResponseString(const string &fileContent, const string &resourceType, const string &statusCode, const string &statusMessage);
    static string createAutoIndexHtml(string path);
    static HttpResponse createHttpResponse(HttpRequest &request);

    void initHttpResponseSelf(string content, string resourceType, int statusCode);
    void initCGIResponse(string cgiPath, string fileToHandle, HttpRequest request);
    void initErrorHttpResponse(int statusCode);

    void handleGetResponse(HttpRequest &request, ServerBlock *serverBlock);
    void handlePostRequest(HttpRequest &request, ServerBlock *serverBlock);

    HttpResponse(HttpRequest &request, ServerBlock *ServerBlock);

    HttpResponse(int code, const string& message, const string& detail, const string& url, const string& reqMethod);
    HttpResponse(int code);


};

#endif // HTTP_RESPONSE_HPP