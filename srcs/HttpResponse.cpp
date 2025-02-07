#include "HttpResponse.hpp"
#include "StatusHandler.hpp"
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

string HttpResponse::decideCGIToUse(string resourcePath) {
    stringDict CGIHandling;
    CGIHandling[".csv"] = "csv_handler.py";

    // check if cgi handling is required
    for (stringDict::const_iterator it = CGIHandling.begin(); it != CGIHandling.end(); ++it) {
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

string HttpResponse::createHttpResponseString(
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

void HttpResponse::setHttpResponseSelf(string content, string resourceType, int statusCode) {
    this->rawContent = content;
    this->contentType = resourceType;
    this->statusCode = statusCode;
    this->message = StatusHandler::CodeToMessage(statusCode);
    this->finalResponseMsg = createHttpResponseString(content, resourceType, to_string(statusCode), message);
}


HttpResponse createErrorHTTPResponse(int statusCode) {
    string resourcePath = "/home/cooper/coreProgram/qi_ter_webserv/public/error.html";
    
    string fileContent = StatusHandler(statusCode).generateErrorPage(resourcePath);
    
    
    string statusMessage = StatusHandler::CodeToMessage(statusCode);
    std::cout << "message: " << RED << fileContent << RESET << std::endl;
    return HttpResponse(fileContent, HttpResponse::getContentType(resourcePath), statusCode);
}

string HttpResponse::generateAutoIndexHtml(string path) {
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

bool isCGI(const string& resourcePath) {
    return endsWith(resourcePath, ".py");
}

HttpResponse::HttpResponse(string content, string contentType, int statusCode) {
    this->rawContent = content;
    this->contentLength = contentLength;
    this->contentType = contentType;
    this->statusCode = statusCode;
    this->message = StatusHandler::CodeToMessage(statusCode);
    this->finalResponseMsg = createHttpResponseString(content, contentType, to_string(statusCode), message);
}

void HttpResponse::callCGIResponse(string cgiPath, string fileToHandle, HttpRequest request) {
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

    this->setHttpResponseSelf(response_content, CONTENT_TYPE_HTML, 200);
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
    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        if (startsWith(path, it->getUri()) && (locationBlock == NULL || it->getUri().size() > locationBlock->getUri().size())) {
            locationBlock = &(*it);
        }
    }
    
    return locationBlock;
}

void HttpResponse::initRedirectResponse(string & redirectUrl, int statusCode) {
    this->setHttpResponseSelf("", CONTENT_TYPE_HTML, statusCode);
    this->finalResponseMsg += "Location: " + redirectUrl + "\r\n";
}


HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) {
    string path = request.getParam("path");
    
    this->_serverBlockRef = serverBlock;
    this->_locationBlockRef = getRelevantLocationBlock(path, serverBlock);

    std::vector<std::string> limitExcept = this->_locationBlockRef->getLimitExcept();

    try {
        // std::pair<int, std::string> redirectInfo = this->_locationBlockRef->getReturn();
        // if (!redirectInfo.second.empty()) {
        //     this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
        //     return;
        // }

        if (std::find(limitExcept.begin(), limitExcept.end(), request.getMethod()) == limitExcept.end())
            throw HttpException(405);

        string reroutedPath = reroutePath(path);
        string content = readFileContent(reroutedPath);

        if (isDirectory(reroutedPath)) {
            string indexFile = containsIndexFile(reroutedPath);
            
            if (!indexFile.empty()) {
                try {
                    this->setHttpResponseSelf(readFileContent(indexFile), CONTENT_TYPE_HTML, 200);
                } catch (const HttpException& e) { }
                
                return;
            }
            
            else if (serverBlock->getAutoindex()) {
                string autoIndex = generateAutoIndexHtml(reroutedPath);
                this->setHttpResponseSelf(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            else {
                throw HttpException(403);
            }
        }
        
        else {
            // Check if CGI handling is required for the resource
            string cgiToUse = decideCGIToUse(path);

            // If CGI handling is required, reroute the path to the CGI script
            if (!cgiToUse.empty())
                this->callCGIResponse(cgiToUse, path, request);
            else if (isCGI(path))
                this->callCGIResponse(path, request.getParam("path_info"), request);
            else
                this->setHttpResponseSelf(content, getContentType(reroutedPath), 200);

            std::cout << this->getFinalResponseMsg() << std::endl;
        }
    } 

    catch (const HttpException& e) {
        std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
        this->initErrorHttpResponse(e.getStatusCode());
    }
}


void HttpResponse::initErrorHttpResponse(int statusCode) {
    if (!this->_locationBlockRef->getErrorPage().empty() && 
        this->_locationBlockRef->getErrorPage().find(statusCode) != this->_locationBlockRef->getErrorPage().end()) {
        string errorPage = reroutePath(this->_locationBlockRef->getErrorPage()[statusCode]);

        try {
            this->setHttpResponseSelf(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
        } catch (const HttpException& e) {
            this->setHttpResponseSelf(StatusHandler(500).generateErrorPage("public/error.html"), CONTENT_TYPE_HTML, 500);
        }
    } 
    
    else {
        this->setHttpResponseSelf(StatusHandler(statusCode).generateErrorPage("public/error.html"), CONTENT_TYPE_HTML, statusCode);
    }
}


/*
note: the program should handle hte case in which the cgi contains content length in its header

The CGI's output, if it provides a Content-Length, would look like this:

Content-Length: 48  <-- Important: newline after 48!
                   <-- Blank line
<h1>Hello from CGI!</h1>
<p>This is some dynamic content.</p>


how should the POST method be handled? 


*/