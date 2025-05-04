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
    static const string css;

    string        rawContent;
    int           contentLength;
    int           statusCode;
    string        message;
    string        contentType;
    string        finalResponseMsg;
    bool          isLocation;
    
    int           httpStatusCode;       // HTTP status code (e.g., 404, 500)
    string        errorMessage;      // Short error message
    string        details;           // Detailed explanation
    string        requestUrl;        // URL that caused the error
    string        method;            // HTTP method (GET, POST, etc.)
    std::time_t   timestamp;    // Error occurrence time
    
    string        _absolutePath;

    LocationBlock *emptyBlock;
    ServerBlock   *_serverBlockRef;
    Block         *_locationBlockRef;

    string         containsIndexFile(string path);
    LocationBlock *resolveLocationBlock(const string &path, ServerBlock *serverBlock);
    void          initRedirectResponse(string &redirectUrl, int statusCode);
    string        applyAlias(string &path);
    std::pair<string, string> GetMsg(int statusCode) const;
    std::pair<string, string> CodeToMessage(int statusCode, string message="", string description="") const;
    std::vector<LocationBlock>::iterator getRelevantLocationBlock(ServerBlock *serverBlock, string path);
    string        createStatusPageStr(string errorPagePath, int statusCode) const;
    void          displayError() const;
    HttpRequest &request;


public:
    /* CONSTRUCTOR/DESTRUCTOR */

    HttpResponse(HttpRequest &request, ServerBlock *ServerBlock);


    /* GETTERS/SETTER */
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
    string getAbsolutePath() const;
    string getTimestamp() const;

    string composeHttpResponse(
        const string& body,
        int statusCode,
        const string& msg,
        ... );

    LocationBlock *getBlock();
    bool isCGI(const string &resourcePath);
    string decideCGIToUse(string resourcePath);
    string reroutePath(string urlPath);
    string createAutoIndexHtml(string path, string root);
    static string getContentType(const string& resourcePath);


    /* METHOD HANDLERS */
    void handleGET(ServerBlock *serverBlock);
    void handlePOST();
    void handleDELETE(string path);


    /* RESPONSE INITIALIZERS */
    void initHttpResponse(string content, string resourceType, int statusCode);
    void initErrorHttpResponse(int statusCode, string error = "", string description = "");
    void initCGIResponse(string cgiPath, HttpRequest request);
};

#endif 