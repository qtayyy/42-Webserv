#include "HttpResponse.hpp"
#include "Utils.hpp"

/* DEFAULT VALUES */

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
const string HttpResponse::css = 
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

        std::cout << "REROUTED PATH: " << path << std::endl;


        if (!isPathExist(reroutedPath)) {
            throw HttpException(404, reroutedPath);
        }

        // if the path is a directory
        if (isDirectory(reroutedPath)) {
            std::cout << "Directory detected" << std::endl;
            string indexFile = containsIndexFile(reroutedPath);
            
            // if index file is found, serve it
            if (!indexFile.empty()) {
                try {
                    this->initHttpResponse(readFileContent(indexFile), CONTENT_TYPE_HTML, 200);
                } catch (const HttpException& e) { }
                
                return;
            }
            
            // if autoindex is enabled, serve the autoindex page
            else if (serverBlock->getAutoindex()) {
                std::cout << "Generating auto index... " << reroutedPath<< std::endl;
                string autoIndex = createAutoIndexHtml(reroutedPath, path);
                this->initHttpResponse(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            // if autoindex is disabled, return 403
            else {
                throw HttpException(403);
            }
        }
        
        // if the path is a file
        else {
            string content = readFileContent((reroutedPath));

            this->initHttpResponse(content, getContentType(reroutedPath), 200);
        }
    } 

    catch (const HttpException& e) {
        std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
        this->initErrorHttpResponse(e.getStatusCode());
    }
}


void HttpResponse::handlePOST(HttpRequest &request) {
    string cgiPass = this->getBlock()->getCgiPass();
    this->initCGIResponse(cgiPass, request);
}


void HttpResponse::handleDELETE(string path) {
    path = reroutePath(path);
    if (!isPathExist(path)) {
        this->initErrorHttpResponse(404);
        std::cout << "FILE DOESN'T EXIST: " << path << std::endl;
        return; 
    } 

    int status = remove(path.c_str());

    if (status == 0) {
        this->initHttpResponse("File deleted successfully", CONTENT_TYPE_HTML, 200);
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
    this->_locationBlockRef = resolveLocationBlock(path, serverBlock);


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
    std::pair<int, string> redirectInfo = this->getBlock()->getReturn();
    if (!redirectInfo.second.empty()) {
        std::cout << "redirecting to :" << redirectInfo.second << std::endl;
        this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
        return;
    }

    if (!isPathExist(this->getBlock()->getRoot())) {
        this->initErrorHttpResponse(404);
    }


    /* HANDLE METHODS */

    if (method == "GET") {
        std::cout << "Handling GET..." << std::endl;
        this->handleGET(request, serverBlock);
    }

    else if (method == "POST") {
        std::cout << "Handling POST..." << std::endl;
        this->handlePOST(request);
    }

    else if (method == "DELETE") {
        std::cout << "Handling DELETE..." << std::endl;
        this->handleDELETE(path);
    }

    /* INVALID METHOD */

    else {
        this->initErrorHttpResponse(400);
    }
    std::cout << std::endl;
}



/* GENERATORS */

string HttpResponse::createStatusPageStr(string errorPagePath, int statusCode) const {
    string errorFileContents = readFileContent(errorPagePath);
        
    std::pair<string, string> details = CodeToMessage(statusCode);

    replaceIfFound(&errorFileContents, "{{ERROR_CODE}}", to_string(statusCode));
    replaceIfFound(&errorFileContents, "{{ERROR_MESSAGE}}", details.first);
    replaceIfFound(&errorFileContents, "{{DETAILS}}", details.second);
    
    return errorFileContents;
}

string HttpResponse::reroutePath(string urlPath) {

    string reroutedPath = joinPaths(this->_serverBlockRef->getRoot(), urlPath);

    if (this->isLocation)
        reroutedPath = joinPaths(this->getBlock()->getRoot(), urlPath);

    return reroutedPath;
}


string HttpResponse::createAutoIndexHtml(string path, string root) {
    std::stringstream output;
    output << "<html><head><title>Index of " << path << "</title>"
           << "<style>" << HttpResponse::css << "</style></head><body>"
           << "<h1>Index of " << path << "</h1><table>"
           << "<tr><th>Name</th><th>Size</th></tr>";

    stringList files = listFiles(path);
    if (!files.empty()) {
        for (stringList::iterator it = files.begin(); it != files.end(); ++it) {
            if (*it == "." || *it == "..")
                continue;
            
            string fullPath = joinPaths(path, *it);
            string fileSize = isDirectory(fullPath) ? "-" : to_string(getFileSize(fullPath));

            fullPath = "http://localhost:1234" + joinPaths(root, *it);

            if (isDirectory(fullPath))
                output << "<tr><td><a class=\"folder\" href=\"" << fullPath << "\">" << *it << "</a></td><td>" << fileSize << "bytes</td></tr>";
            else
                output << "<tr><td><a href=\"" << fullPath << "\">" << *it << "</a></td><td>" << fileSize << "</td></tr>";
        }
    } 
    
    else {
        output << "<tr><td>Unable to open directory</td></tr>";
    }

    output << "</table></body></html>";
    return output.str();
}



/* GETTERS */

string HttpResponse::getFinalResponseMsg() const { return finalResponseMsg; }
int    HttpResponse::getStatusCode()       const { return httpStatusCode; }
string HttpResponse::getRequestUrl()       const { return requestUrl; }
string HttpResponse::getMethod()           const { return method; }
int    HttpResponse::getContentLength()    const { return contentLength; }




string HttpResponse::composeHttpResponse(
    const string& body,
    int statusCode,
    const string& msg,
    ... ) 
{
    std::map<string, string> headers;
    va_list args;
    va_start(args, msg);

    while (true) {
        const char* key = va_arg(args, const char*);
        if (!key) break; // NULL signals end of pairs
        const char* value = va_arg(args, const char*);
        if (!value) break; // malformed input
        headers[key] = value;
    }

    va_end(args);

    // Construct response
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << msg << "\r\n";
    for (stringDict::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n" << body;

    return response.str();
}


/* INITIALIZERS */

void HttpResponse::initHttpResponse(string body, string resourceType, int statusCode) {
    this->rawContent       = body;
    this->contentType      = resourceType;
    this->statusCode       = statusCode;
    this->message          = CodeToMessage(statusCode).second;
    // this->finalResponseMsg = composeResponse(body, resourceType, to_string(statusCode), message);
    this->finalResponseMsg = composeHttpResponse(body, statusCode, message, 
                                "Content-Length",  to_string(body.size()).c_str(),
                                "Connection",      "close",
                                "Content-Type",    resourceType.c_str(),
                                NULL);
}



void HttpResponse::initRedirectResponse(string & redirectUrl, int statusCode) {
    this->rawContent    = "";
    this->contentType   = CONTENT_TYPE_HTML;
    this->statusCode    = statusCode;
    this->message       = CodeToMessage(statusCode).second;
    
    this->finalResponseMsg = composeHttpResponse("", statusCode, message,
        "Location",        redirectUrl.c_str(),
        "Content-Length",  "0",
        "Content-Type",    CONTENT_TYPE_HTML,
        NULL);
}


void HttpResponse::initErrorHttpResponse(int statusCode, string error, string description) {
    if (!this->_locationBlockRef->getErrorPage().empty() && 
        this->_locationBlockRef->getErrorPage().find(statusCode) != this->_locationBlockRef->getErrorPage().end()) {
        try {
            string errorPage = this->_locationBlockRef->getErrorPage()[statusCode];
            std::cout << "error page: " << errorPage << "'" << std::endl;
            this->initHttpResponse(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
            return ;
        }
        catch (const HttpException& e) {
            statusCode = 500;
            std::cout << "exception" << e.getStatusCode() << std::endl;
        }
    }

    std::pair<string, string> details = CodeToMessage(statusCode);
    
    string errorMsg = details.first;
    string errorDescription = details.second;
    
    if (error != "")       errorMsg = error;
    if (description != "") errorDescription = description;
    
    std::stringstream output;
    output  << "<html><head><title>" << errorMsg << "</title>"
            << "<style>" << HttpResponse::css << "</style></head><body>"
            << "<h1 class=\"error\">" << errorMsg << "</h1>"
            << "<p>" << errorDescription << "</p>"
            << "</body></html>";

    this->initHttpResponse(output.str(), CONTENT_TYPE_HTML, statusCode);
}


void HttpResponse::initCGIResponse(string cgiPath, HttpRequest request) {

    std::cout << YELLOW << "Initializing CGI: \"" << cgiPath << "\"" RESET << std::endl;

    cgiPath = getAbsolutePath(cgiPath);
    if (!isPathExist(cgiPath)) {
        std::cout << "path doesn't exist: " << cgiPath << std::endl;
        this->initErrorHttpResponse(500);
        return;
    }

    string pathToHandle = request.headerGet("path_info");

    CGIHandler cgiHandler = CGIHandler();
    string absolutePath = getAbsolutePath(pathToHandle);

    request.headerSet("absolute_path", absolutePath);

    int exit_status = 0;
    string response_content = cgiHandler.handleCgi(cgiPath, request, exit_status, *this->getBlock());
   
    std::cout << "LOCATION BLOCK" << this->getBlock()->getInfo() << std::endl;

    this->finalResponseMsg = response_content;
}



/* OTHERS */

string HttpResponse::getContentType(const string& resourcePath) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        if (endsWith(resourcePath, it->first))
            return it->second;
    }
    
    return "text/plain";
}

string HttpResponse::containsIndexFile(string path) {
    const std::vector<string>& indices = this->_serverBlockRef->getIndex();
    for (std::vector<string>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
        if (isPathExist(path + "/" + *it)) {
            return path + "/" + *it;
        }
    }

    return "";
}

LocationBlock* HttpResponse::resolveLocationBlock(const string& path, ServerBlock* serverBlock) {
    std::vector<LocationBlock>* locations = serverBlock->getLocation();
    LocationBlock* locationBlock = NULL;

    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        const string& uri = it->getUri();

        if (startsWith(path, uri) &&
            (path.size() == uri.size() || path[uri.size()] == '/') &&
            (locationBlock == NULL || uri.size() > locationBlock->getUri().size()))
        {
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

std::pair<string, string> HttpResponse::GetMsg(int statusCode) const {
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

std::pair<string, string> HttpResponse::CodeToMessage(int statusCode, string message, string description) const {
    std::pair<string, string> statusMsg = HttpResponse::GetMsg(statusCode);
    if (message != "")
        statusMsg.first = message;
    if (description != "")
        statusMsg.second = description;

    return statusMsg;
}

