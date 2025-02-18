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
    map[".pdf"] = CONTENT_TYPE_PDF;
    return map;
}

const stringDict HttpResponse::contentTypeMap = HttpResponse::createContentTypeMap();



/* CONSTRUCTORS */

LocationBlock *HttpResponse::getBlock() {
    if (this->_locationBlockRef) {
        return dynamic_cast<LocationBlock*>(this->_locationBlockRef);
    } else {
        return this->emptyBlock;
    }
}


/* REQUEST HANDLERS */

void HttpResponse::handleGET(HttpRequest &request, ServerBlock *serverBlock) {
    string path = request.headerGet("path");
    
    path = applyAlias(path);
    
    // Check if a redirect is required
    std::pair<int, std::string> redirectInfo = this->getBlock()->getReturn();
    if (!redirectInfo.second.empty()) {
        std::cout << "redirecting to :" << redirectInfo.second << std::endl;
        this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
        return;
    }

    if (!this->getBlock()->getCgiPass().empty()) {
        this->initCGIResponse(this->getBlock()->getCgiPass(), request);
        return;
    }

    try {

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
            // if (!cgiToUse.empty())
            //     this->initCGIResponse(cgiToUse, request);
            // else if (isCGI(path))
            //     this->initCGIResponse(path, request);
            // else
            this->initHttpResponseSelf(content, getContentType(reroutedPath), 200);

            // std::cout << this->getFinalResponseMsg().substr(100) << std::endl;
        }
    } 

    catch (const HttpException& e) {
        std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
        this->initErrorHttpResponse(e.getStatusCode());
    }
}


void HttpResponse::handlePOST(HttpRequest &request, ServerBlock *serverBlock) {
    string cgiPass = this->getBlock()->getCgiPass();

    this->initCGIResponse(cgiPass, request);
}


void HttpResponse::handleDELETE(string path) {
    path = reroutePath(path);
    if (!doesPathExist(path)) {
        this->initErrorHttpResponse(404);
        std::cout << "FILE DOESN'T EXIST: " << path << std::endl;
        return; 
    } 

    int status = remove(path.c_str());

    if (status == 0) {
        this->initHttpResponseSelf("File deleted successfully", CONTENT_TYPE_HTML, 200);
    } 
    
    else {
        std::cout << path << " could not be deleted. Status: " << status << std::endl;
        this->initErrorHttpResponse(500);
    }
}


HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) {
    std::cout << YELLOW << "\nConstructing response..." << std::endl;
    this->emptyBlock = new LocationBlock();
    string path = request.headerGet("path");
    this->_serverBlockRef = serverBlock;
    this->_locationBlockRef = getRelevantLocationBlock(path, serverBlock);


    if (this->_locationBlockRef == NULL) {
        this->_locationBlockRef = serverBlock;
        this->isLocation = false;
        std::cout << YELLOW << "\t" << path << " Does not match any location. Defaulting..." << RESET << std::endl;
    }
    else {
        std::cout << "\tLocation matches location block: " << this->getBlock()->getUri() << std::endl;
    }
    
    this->getBlock()->printBlock();

    stringList limitExcept = this->_locationBlockRef->getLimitExcept();

    // Check if the request method is allowed
    if (std::find(limitExcept.begin(), limitExcept.end(), request.getMethod()) == limitExcept.end()) {
        this->initErrorHttpResponse(405);
        return;
    }

    string method = request.headerGet("method");

    std::cout << YELLOW << "\tHandling method: " << method << RESET << std::endl;
    if (method == "GET") {
        this->handleGET(request, serverBlock);
    } 
    else if (method == "POST") {
        this->handlePOST(request, serverBlock);
    }
    else if (method == "DELETE") {
        this->handleDELETE(path);
    }
    else {
        this->initErrorHttpResponse(400);
    }
    std::cout << std::endl;
}



/* GENERATORS */

string HttpResponse::createStatusPageStr(string errorPagePath, int statusCode) const {
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
    response << PROTOCOL << " " << statusCode << " " << statusMessage << "\n";
    response << "Content-Length: " << fileContent.size() << "\n";
    response << "Connection: " << "close" << "\n";
    // response << "Cache-Control: " << "no-cache, no-store, must-revalidate" << "\n";
    // response << "Pragma: " << "no-cache" << "\n";
    // response << "Expires: " << "0" << "\n";
    response << "Content-Type: " << resourceType << "\n\n";
    response << fileContent;

    return response.str();
}

string Css() {
    return 
"       body {"
"           font-family: Arial, sans-serif;"
"           background-color: #f4f4f4;"
"           margin: 0;"
"           padding: 20px;"
"       }"
"       h1 {"
"           color: #4CAF50;"
"       }"
"       table {"
"           width: 100%;"
"           border-collapse: collapse;"
"           border-radius: 10px;"
"           overflow: hidden;"
"           padding: 10px;"
"       }"
"       th, td {"
"           text-align: left;"
"           background: #fff;"
"           padding: 10px;"
"       }"
"       a {"
"           color: #1E90FF;"
"       }"
"       .folder {"
"           color:rgb(0, 255, 251);"
"       }"
"       .error {"
"           color: red;"
"           font-weight: bold;"
"       }";
}

int getFileSize(const std::string& filePath) {
    struct stat stat_buf;
    int rc = stat(filePath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

string HttpResponse::createAutoIndexHtml(string path) {
    std::stringstream ss;
    ss << "<html><head><title>Index of " << path << "</title>";
    ss << "<style>" << Css() << "</style></head><body>";
    ss << "<h1>Index of " << path << "</h1><table>";
    ss << "<tr><th>Name</th><th>Size</th></tr>"; // Table header

    std::vector<string> files = listFiles(path);
    if (!files.empty()) {
        for (std::vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
            if (*it == "." || *it == "..")
                continue;
            
            std::string fullPath = path + "/" + *it;
            std::string fileSize = isDirectory(fullPath) ? "-" : to_string(getFileSize(fullPath));

            if (isDirectory(fullPath))
                ss << "<tr><td><a class=\"folder\" href=\"" << *it << "\">" << *it << "</a></td><td>" << fileSize << "</td></tr>";
            else
                ss << "<tr><td><a href=\"" << *it << "\">" << *it << "</a></td><td>" << fileSize << "</td></tr>";
        }
    } 
    
    else {
        ss << "<tr><td>Unable to open directory</td></tr>";
    }

    ss << "</table></body></html>";
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
        try {
            string errorPage = this->_locationBlockRef->getErrorPage()[statusCode];
            std::cout << "error page: " << errorPage << "'" << std::endl;
            std::cout << "exist?: " << doesPathExist(errorPage) << std::endl;
            this->initHttpResponseSelf(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
            return ;
        }
        catch (const HttpException& e) {
            statusCode = 500;
            std::cout << "exception" << e.getStatusCode() << std::endl;
        }
    }

    std::pair<std::string, std::string> details = CodeToMessage(statusCode);
    std::stringstream ss;
    ss << "<html><head><title>" << details.first << "</title>";
    ss << "<style>" << Css() << "</style></head><body>";
    ss << "<h1 class=\"error\">" << details.first << "</h1>";
    ss << "<p>" << details.second << "</p>";
    ss << "</body></html>";

    this->initHttpResponseSelf(ss.str(), CONTENT_TYPE_HTML, statusCode);
}

void HttpResponse::initCGIResponse(string cgiPath, HttpRequest request) {

    cgiPath = getAbsolutePath(cgiPath);
    if (!doesPathExist(cgiPath) || this->getBlock()->getCgiPass().empty()) {
        std::cout << "path doesn't exist" << cgiPath << std::endl;
        this->initErrorHttpResponse(500);
        return;
    }

    string pathToHandle = request.headerGet("path_info");

    CGIHandler cgiHandler = CGIHandler();
    string absolutePath = getAbsolutePath(this->getBlock()->getUploadPath() + pathToHandle);

    request.headerSet("absolute_path", absolutePath);

    int exit_status = 0;
    string response_content = cgiHandler.handleCgi(cgiPath, request, exit_status, *this->_serverBlockRef);
   
    this->finalResponseMsg = response_content;
}


bool HttpResponse::isCGI(const string& resourcePath) {
    return endsWith(resourcePath, ".py");
}


string HttpResponse::decideCGIToUse(string resourcePath) {
    // stringDict cgiScripts = this->_locationBlockRef->getCgiScript();

    // check if cgi handling is required
    // for (stringDict::const_iterator it = cgiScripts.begin(); it != cgiScripts.end(); ++it) {
    //     if (endsWith(resourcePath, it->first)) {
    //         return (CGI_BIN_PATH + string("/") + it->second);
    //     }
    // }
    // return "";
	(void)resourcePath;
	return (CGI_BIN_PATH + string("/") + this->getBlock()->getCgiPass());
}

string HttpResponse::getContentType(const string& resourcePath) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        size_t extension_start = resourcePath.size() - it->first.size();

        if (endsWith(resourcePath, it->first))
            return it->second;
    }

    return "text/plain";
}

string HttpResponse::reroutePath(string urlPath) {
    stringDict pathMap;

    pathMap["cgi-bin"] = "";

    string reroutedPath = this->_locationBlockRef->getRoot() + "/" + urlPath;
    for (stringDict::const_iterator it = pathMap.begin(); it != pathMap.end(); ++it) {
        if (startsWith(urlPath, it->first)) {
            reroutedPath = (it->second + urlPath);
        }
    }

    return reroutedPath;
}

HttpResponse::HttpResponse(string content, string contentType, int statusCode) {
    this->rawContent = content;
    this->contentLength = contentLength;
    this->contentType = contentType;
    this->statusCode = statusCode;
    this->message = CodeToMessage(statusCode).second;
    this->finalResponseMsg = createResponseString(content, contentType, to_string(statusCode), message);
}

string HttpResponse::containsIndexFile(string path) {
    const std::vector<std::string>& indices = this->_serverBlockRef->getIndex();
    for (std::vector<std::string>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
        if (doesPathExist(path + "/" + *it)) {
            return path + "/" + *it;
        }
    }

    return "";
}

std::string getDirectory(const std::string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return fullPath.substr(0, pos);
    }
    return "";
}

LocationBlock* HttpResponse::getRelevantLocationBlock(const string& path, ServerBlock* serverBlock) {
    std::vector<LocationBlock> *locations = serverBlock->getLocation();
    LocationBlock* locationBlock = NULL;
    string directory = getDirectory(path);
    
    
    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        if (startsWith(directory, it->getUri()) && (locationBlock == NULL || it->getUri().size() > locationBlock->getUri().size())) {
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
        case 202: return std::make_pair("Accepted", "The request has been accepted for processing, but the processing has not been completed.");
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

