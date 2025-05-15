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
#include "Utils.hpp"

typedef std::map<string, string> stringDict;

class HttpResponse {
private:
    static const stringDict contentTypeMap;
    static stringDict createContentTypeMap();
    static const string css;

    int           _contentLength;
    int           _statusCode;
    string        _message;
    string        _contentType;
    string        _finalResponseMsg;
    bool          _usingLocationBlock;
    string        _path;
    string        _reroutedPath;
    bool          _isAlias;
    bool          _isExtension;
    HttpRequest   &_request;

    LocationBlock _emptyBlock;
    ServerBlock   *_serverBlockRef;
    Block         *_resolvedLocationBlock;


    /* PATH RESOLUTION */
    static string _resolveContentType(const string& resourcePath);
    Block  *_resolveLocationBlock(const string &path, ServerBlock *serverBlock);
    std::vector<LocationBlock>::iterator getRelevantLocationBlock(ServerBlock *serverBlock, string path);
    string _applyAlias(string &path);
    string _applyRoot(string urlPath);
    
    string  _dirHasIndexFile(string path);
    

    /* METHOD HANDLERS */
    void _handleGET();
    void _handlePOST();
    void _handleDELETE();
    

    /* RESPONSE INITIALIZERS */
    void _initRedirectResponse(string &redirectUrl, int statusCode);
    void _initHttpResponse(string content, string resourceType, int statusCode);
    void _initErrorResponse(int statusCode, string error = "", string description = "");
    void _initCGIResponse(string cgiPath, HttpRequest request);


    /*ERRORS*/
    std::pair<string, string> GetMsg(int statusCode) const;
    std::pair<string, string> CodeToMessage(int statusCode, string message="", string description="") const;

public:

    /* CONSTRUCTOR/DESTRUCTOR */
    HttpResponse(HttpRequest &request, ServerBlock *ServerBlock);
    ~HttpResponse();


    /* GETTERS/SETTER */
    int            getContentLength() const;
    string         getFinalResponseMsg() const;
    string         getReroutedPath();
    LocationBlock *getBlock();


    /* GENERATORS */
    static string composeHttpResponse(
        const string& body,
        int statusCode,
        const string msg,
        ... );

    string generateAutoIndexHtml(string path, string root);
};

#endif 