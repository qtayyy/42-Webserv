#include "HttpResponse.hpp"
#include "Utils.hpp"

stringDict HttpResponse::createContentTypeMap() {
    stringDict map;
    map[".html"] = CONTENT_TYPE_HTML;
    map[".css"] = CONTENT_TYPE_CSS;
    map[".js"] = CONTENT_TYPE_JS;
    map[".png"] = CONTENT_TYPE_PNG;
    map[".jpg"] = CONTENT_TYPE_JPG;
    map[".gif"] = CONTENT_TYPE_GIF;
    return map;
}

const stringDict HttpResponse::contentTypeMap = HttpResponse::createContentTypeMap();


/* CONSTRUCTORS */

HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) {
    string path = request.getParam("path");
    
    
    this->_serverBlockRef = serverBlock;
    this->_locationBlockRef = getRelevantLocationBlock(path, serverBlock);
    
    path = applyAlias(path);
    
    stringList limitExcept = this->_locationBlockRef->getLimitExcept();


    try {
        // Check if the request method is allowed
        if (std::find(limitExcept.begin(), limitExcept.end(), request.getMethod()) == limitExcept.end())
            throw HttpException(405);

        // Check if a redirect is required
        std::pair<int, std::string> redirectInfo = this->_locationBlockRef->getReturn();
        if (!redirectInfo.second.empty()) {
            this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
            return;
        }

        string reroutedPath = reroutePath(path);
        string content = readFileContent(reroutedPath);

        // if the path is a directory
        if (isDirectory(reroutedPath)) {
            string indexFile = containsIndexFile(reroutedPath);
            
            // if index file is found, serve it
            if (!indexFile.empty()) {
                try {
                    this->initHttpResponseSelf(readFileContent(indexFile), CONTENT_TYPE_HTML, 200);
                } catch (const HttpException& e) { }
                
                return;
            }
            
            // if autoindex is enabled, serve the autoindex page
            else if (serverBlock->getAutoindex()) {
                string autoIndex = createAutoIndexHtml(reroutedPath);
                this->initHttpResponseSelf(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            // if autoindex is disabled, return 403
            else {
                throw HttpException(403);
            }
        }
        
        // if the path is a file
        else {
            // Check if CGI handling is required for the resource
            string cgiToUse = decideCGIToUse(path);

            // If CGI handling is required, reroute the path to the CGI script
            if (!cgiToUse.empty())
                this->initCGIResponse(cgiToUse, path, request);
            else if (isCGI(path))
                this->initCGIResponse(path, request.getParam("path_info"), request);
            else
                this->initHttpResponseSelf(content, getContentType(reroutedPath), 200);

            std::cout << this->getFinalResponseMsg() << std::endl;
        }
    } 

    catch (const HttpException& e) {
        std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
        this->initErrorHttpResponse(e.getStatusCode());
    }
}


/* GENERATORS */

string HttpResponse::createErrorPage(string errorPagePath, int statusCode) const {
    string errorFileContents = readFileContent(errorPagePath);
        
    std::pair<std::string, std::string> details = CodeToMessage(statusCode);

    replaceIfFound(&errorFileContents, "{{ERROR_CODE}}", to_string(statusCode));
    replaceIfFound(&errorFileContents, "{{ERROR_MESSAGE}}", details.first);
    replaceIfFound(&errorFileContents, "{{DETAILS}}", details.second);
    
    return errorFileContents;
}


string HttpResponse::createResponseString(
    const string& fileContent, 
    const string& resourceType, 
    const string& statusCode, 
    const string& statusMessage) {

    std::stringstream response;
    response << PROTOCOL << " " << statusCode << " " << statusMessage << "\r\n";
    response << "Content-Length: " << fileContent.size() << "\r\n";
    response << "Content-Type: " << resourceType << "\r\n\r\n";
    response << fileContent;

    return response.str();
}


string HttpResponse::createAutoIndexHtml(string path) {
    std::stringstream ss;
    ss << "<html><head><title>Index of " << path << "</title></head><body>";
    ss << "<h1>Index of " << path << "</h1><ul>";

    DIR* dir = opendir(path.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            if (name != "." && name != "..") {
                ss << "<li><a href=\"" << name << "\">" << name << "</a></li>";
            }
        }
        closedir(dir);
    } else {
        ss << "<li>Unable to open directory</li>";
    }

    ss << "</ul></body></html>";
    return ss.str();
}


/* GETTERS */

string HttpResponse::getFinalResponseMsg() const { return finalResponseMsg; }
int    HttpResponse::getStatusCode()       const { return httpStatusCode; }
string HttpResponse::getRequestUrl()       const { return requestUrl; }
string HttpResponse::getMethod()           const { return method; }
int    HttpResponse::getContentLength()    const { return contentLength; }


/* INITIALIZERS */

void HttpResponse::initHttpResponseSelf(string content, string resourceType, int statusCode) {
    this->rawContent       = content;
    this->contentType      = resourceType;
    this->statusCode       = statusCode;
    this->message          = CodeToMessage(statusCode).second;
    this->finalResponseMsg = createResponseString(content, resourceType, to_string(statusCode), message);
}

void HttpResponse::initRedirectResponse(string & redirectUrl, int statusCode) {
    this->rawContent = "";
    this->contentType = CONTENT_TYPE_HTML;
    this->statusCode = statusCode;
    this->message = CodeToMessage(statusCode).second;
    
    std::stringstream response;
    response << PROTOCOL << " " << statusCode << " " << message << "\r\n";
    response << "Location: " + redirectUrl + "\r\n";
    response << "Content-Length: 0\r\n";
    response << "Content-Type: " << CONTENT_TYPE_HTML << "\r\n\r\n";
    
    this->finalResponseMsg = response.str();
}

void HttpResponse::initErrorHttpResponse(int statusCode) {
    if (!this->_locationBlockRef->getErrorPage().empty() && 
        this->_locationBlockRef->getErrorPage().find(statusCode) != this->_locationBlockRef->getErrorPage().end()) {
        string errorPage = reroutePath(this->_locationBlockRef->getErrorPage()[statusCode]);

        try {
            this->initHttpResponseSelf(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
        } catch (const HttpException& e) {
            this->initHttpResponseSelf(createErrorPage("public/error.html", 500), CONTENT_TYPE_HTML, 500);
        }
    } 
    
    else {
        this->initHttpResponseSelf(createErrorPage("public/error.html", 500), CONTENT_TYPE_HTML, statusCode);
    }
}

void HttpResponse::initCGIResponse(string cgiPath, string fileToHandle, HttpRequest request) {
    string absolutePath = getAbsolutePath(this->reroutePath(fileToHandle));

    cgiPath = getAbsolutePath(cgiPath);
    request.printInfo();

    CGIHandler cgiHandler = CGIHandler();

    std::cout << GREEN << "Query string: " << request.getParam("query_string") << RESET << std::endl;
    cgiHandler.setEnv("REQUEST_METHOD",  request.getMethod());
    cgiHandler.setEnv("QUERY_STRING",    request.getParam("query_string"));
    cgiHandler.setEnv("SCRIPT_NAME",     cgiPath);
    cgiHandler.setEnv("SERVER_NAME",     "localhost");
    cgiHandler.setEnv("SERVER_PORT",     "8080");
    cgiHandler.setEnv("PATH_INFO",       fileToHandle);
    cgiHandler.setEnv("PATH_TRANSLATED", absolutePath);

    int exit_status = 0;
    string response_content = cgiHandler.handleCgiRequest(cgiPath, request.getParam("query_string"), absolutePath, exit_status);
        // Check if the response contains the content length in its header, if so, remove the header
    if (startsWith(response_content, "Content-Length: ") && response_content.find("\n\n") != string::npos) {
        response_content = response_content.substr(response_content.find("\n\n") + 2);
    }

    this->initHttpResponseSelf(response_content, CONTENT_TYPE_HTML, 200);
}


bool HttpResponse::isCGI(const string& resourcePath) {
    return endsWith(resourcePath, ".py");
}


string HttpResponse::decideCGIToUse(string resourcePath) {
    stringDict cgiScripts = this->_locationBlockRef->getCgiScript();

    // check if cgi handling is required
    for (stringDict::const_iterator it = cgiScripts.begin(); it != cgiScripts.end(); ++it) {
        if (endsWith(resourcePath, it->first)) {
            return (CGI_BIN_PATH + string("/") + it->second);
        }
    }
    return "";
}

string HttpResponse::getContentType(const string& resourcePath) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        size_t extension_start = resourcePath.size() - it->first.size();

        if (endsWith(resourcePath, it->first))
            return it->second;
    }

    return "text/plain";
}

string HttpResponse::reroutePath(const string &urlPath) {
    stringDict pathMap;

    pathMap["cgi-bin"] = "";

    string reroutedPath = this->_locationBlockRef->getRoot() + "/" + urlPath;
    for (stringDict::const_iterator it = pathMap.begin(); it != pathMap.end(); ++it) {
        if (startsWith(urlPath, it->first)) {
            reroutedPath = (it->second + urlPath);
        }
    }

    std::cout << "rerouted path" << RED << reroutedPath << RESET << std::endl;

    if (!doesPathExist(reroutedPath)) {
        return "";
    }

    return reroutedPath;
}


string HttpResponse::containsIndexFile(string path) {
    const std::vector<std::string>& indices = this->_locationBlockRef->getIndex();
    for (std::vector<std::string>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
        if (doesPathExist(path + "/" + *it)) {
            return path + "/" + *it;
        }
    }


    return "";
}


LocationBlock* HttpResponse::getRelevantLocationBlock(const string& path, ServerBlock* serverBlock) {
    std::vector<LocationBlock> *locations = serverBlock->getLocation();
    LocationBlock* locationBlock = NULL;
    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        if (startsWith(path, it->getUri()) // if the path starts with the location block's uri
            && (locationBlock == NULL || it->getUri().size() > locationBlock->getUri().size())) {
            locationBlock = &(*it);
        }
    }
    
    return locationBlock;
}


string HttpResponse::applyAlias(string& path) {
    std::vector<LocationBlock> *locations = this->_serverBlockRef->getLocation();
    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        string alias = it->getAlias();
        if (!alias.empty() && startsWith(path, it->getUri())) {
            string newPath = alias + path.substr(it->getUri().length());
            return newPath;
        }
    }
    
    return path;
}


std::pair<std::string, std::string> HttpResponse::CodeToMessage(int statusCode) const {
    switch (statusCode) {
        case 200: return std::make_pair("OK", "The request has succeeded.");
        case 201: return std::make_pair("Created", "The request has been fulfilled and resulted in a new resource being created.");
        case 400: return std::make_pair("Bad Request", "The server could not understand the request due to invalid syntax.");
        case 401: return std::make_pair("Unauthorized", "The client must authenticate itself to get the requested response.");
        case 405: return std::make_pair("Method Not Allowed", "The request method is known by the server but has been disabled and cannot be used.");
        case 403: return std::make_pair("Forbidden", "The client does not have access rights to the content, i.e., they are unauthorized.");
        case 404: return std::make_pair("Not Found", "The server can not find the requested resource.");
        case 500: return std::make_pair("Internal Server Error", "Something went wrong on the server.");
        case 504: return std::make_pair("Gateway Timeout", "The server, while acting as a gateway or proxy, did not receive a timely response from an upstream server it needed to access in order to complete the request.");
    }
    return std::make_pair("Unknown", "Unknown error code.");
}

