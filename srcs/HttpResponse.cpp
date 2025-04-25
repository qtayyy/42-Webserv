#include "HttpResponse.hpp"
#include "Utils.hpp"

stringDict HttpResponse::createContentTypeMap() {
    stringDict map;
    map[".html"] = CONTENT_TYPE_HTML;
    map[".css"]  = CONTENT_TYPE_CSS;
    map[".js"]   = CONTENT_TYPE_JS;
    map[".png"]  = CONTENT_TYPE_PNG;
    map[".jpg"]  = CONTENT_TYPE_JPG;
    map[".gif"]  = CONTENT_TYPE_GIF;
    map[".pdf"]  = CONTENT_TYPE_PDF;
    return map;
}

const stringDict HttpResponse::contentTypeMap = HttpResponse::createContentTypeMap();



/* CONSTRUCTORS */

LocationBlock *HttpResponse::getBlock() {
    if (this->isLocation) {
        return dynamic_cast<LocationBlock*>(this->_locationBlockRef);
    } else {
        return LocationBlock::emptyBlock;
        // return this->emptyBlock;
    }
}


/* REQUEST HANDLERS */

void HttpResponse::handleGET(HttpRequest &request, ServerBlock *serverBlock) {
    string path = request.headerGet("path");
    
    path = applyAlias(path);

    if (!this->getBlock()->getCgiPass().empty()) {
        this->initCGIResponse(this->getBlock()->getCgiPass(), request);
        return;
    }

    try {
        string reroutedPath = urlDecode(reroutePath(path));

        if (!doesPathExist(reroutedPath)) {
            throw HttpException(404, reroutedPath);
        }

        // if the path is a directory
        if (isDirectory(reroutedPath)) {
            std::cout << "Directory detected" << std::endl;
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
                std::cout << "Generating auto index... " << std::endl;
                string autoIndex = createAutoIndexHtml(reroutedPath, path);
                this->initHttpResponseSelf(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            // if autoindex is disabled, return 403
            else {
                throw HttpException(403);
            }
        }
        
        // if the path is a file
        else {
            string content = readFileContent((reroutedPath));
            std::cout << "FILE CONTENT FROM: " << reroutedPath << content << std::endl;

            this->initHttpResponseSelf(content, getContentType(reroutedPath), 200);
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



/* CONSTRUCTOR */

HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) {
    std::cout << YELLOW << "\nConstructing response..." << std::endl;
    
    this->emptyBlock        = new LocationBlock();
    string path             = request.headerGet("path");
    this->_serverBlockRef   = serverBlock;
    this->_locationBlockRef = getRelevantLocationBlock(path, serverBlock);


    if (this->_locationBlockRef == NULL) {
        std::cout << YELLOW << path << " Does not match any location. Defaulting..." << RESET << std::endl;
        this->_locationBlockRef = serverBlock;
        this->isLocation = false;
        std::cout << BLUE;
        printBorderedBox(_locationBlockRef->getInfo(), "Using block");
        std::cout << RESET;
    }
    else {
        this->isLocation = true;
        std::cout << "Location matches location block: " << this->getBlock()->getUri() << std::endl;
        printBorderedBox(this->getBlock()->getInfo(), "Using block");
    }
    

    stringList limitExcept = this->_locationBlockRef->getLimitExcept();

    // Check if the request method is allowed
    if (std::find(limitExcept.begin(), limitExcept.end(), request.getMethod()) == limitExcept.end()) {
        this->initErrorHttpResponse(405);
        return;
    }

    string method = request.getMethod();

    // Check if a redirect is required
    std::pair<int, std::string> redirectInfo = this->getBlock()->getReturn();
    if (!redirectInfo.second.empty()) {
        std::cout << "redirecting to :" << redirectInfo.second << std::endl;
        this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
        return;
    }

    
    if (method == "GET") {
        std::cout << "Handling GET..." << std::endl;
        this->handleGET(request, serverBlock);
    }

    else if (method == "POST") {
        std::cout << "Handling POST..." << std::endl;
        this->handlePOST(request, serverBlock);
    }

    else if (method == "DELETE") {
        std::cout << "Handling DELETE..." << std::endl;
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



string HttpResponse::reroutePath(string urlPath) {
    string reroutedPath = appendPaths(this->_serverBlockRef->getRoot(), urlPath);

    if (this->isLocation)
        reroutedPath = appendPaths(this->getBlock()->getRoot(), urlPath);

    return reroutedPath;
}

string HttpResponse::createAutoIndexHtml(string path, string root) {
    std::stringstream ss;
    ss << "<html><head><title>Index of " << path << "</title>";
    ss << "<style>" << Css() << "</style></head><body>";
    ss << "<h1>Index of " << path << "</h1><table>";
    ss << "<tr><th>Name</th><th>Size</th></tr>"; // Table header

    stringList files = listFiles(path);
    if (!files.empty()) {
        for (stringList::iterator it = files.begin(); it != files.end(); ++it) {
            if (*it == "." || *it == "..")
                continue;
            
            string fullPath = appendPaths(path, *it);

            std::cout << "PATHS: " << path << std::endl;
            std::cout << "PATHS: " << *it << std::endl;
            std::cout << "PATHS: " << root << std::endl;

            string fileSize = isDirectory(fullPath) ? "-" : to_string(getFileSize(fullPath));

            fullPath = "http://localhost:1234" + root + "/" + *it;

            if (isDirectory(fullPath))
                ss << "<tr><td><a class=\"folder\" href=\"" << fullPath << "\">" << *it << "</a></td><td>" << fileSize << "bytes</td></tr>";
            else
                ss << "<tr><td><a href=\"" << fullPath << "\">" << *it << "</a></td><td>" << fileSize << "</td></tr>";
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

    std::cout << YELLOW << "Initializing CGI: " << cgiPath << RESET << std::endl;

    cgiPath = getAbsolutePath(cgiPath);
    if (!doesPathExist(cgiPath)) {
        std::cout << "path doesn't exist: " << cgiPath << std::endl;
        this->initErrorHttpResponse(500);
        return;
    }

    string pathToHandle = request.headerGet("path_info");

    CGIHandler cgiHandler = CGIHandler();
    string absolutePath = getAbsolutePath(pathToHandle);

    request.headerSet("absolute_path", absolutePath);

    int exit_status = 0;
    string response_content = cgiHandler.handleCgi(cgiPath, request, exit_status, *this->_serverBlockRef);
   
    this->finalResponseMsg = response_content;
}



/* OTHERS */

string HttpResponse::getContentType(const string& resourcePath) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        size_t extension_start = resourcePath.size() - it->first.size();

        if (endsWith(resourcePath, it->first))
            return it->second;
    }
    
    return "text/plain";
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
        case 200: return std::make_pair("OK", "Success");
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

