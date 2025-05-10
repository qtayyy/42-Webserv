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
"           color:rgb(0, 160, 158);"
"       }"
"       .error {"
"           color: red;"
"           font-weight: bold;"
"       }";


/* CONSTRUCTORS */

LocationBlock *HttpResponse::getBlock() {
    if (this->_isLocation) {
        return dynamic_cast<LocationBlock*>(this->_locationBlockRef);
    } else {
        return LocationBlock::emptyBlock;
        // return this->emptyBlock;
    }
}


/* REQUEST HANDLERS */

void HttpResponse::handleGET(ServerBlock *serverBlock) {
    (void)serverBlock;


    //fixme /custom_error turns into /volatile, but then reroutes to public/volatile/te
    //fixme check if alias applies. If so, don't reroute using root
    if (!this->getBlock()->getCgiPass().empty()) {
        this->initCGIResponse(this->getBlock()->getCgiPass(), request);
        return;
    }

    try {

        // if the _path is a directory
        if (!_reroutedPath.empty() && _reroutedPath[0] == '/') {
            _reroutedPath = _reroutedPath.substr(1);
        }
        if (isDirectory(_reroutedPath)) { //fixme problem here. Doesn't detect an alias as directory
            std::cout << "Directory detected" << std::endl;
            string indexFile = _dirHasIndexFile(_reroutedPath); // fixme issue here also
            
            stringList indices;

            // if index file is found, serve it
            indices = getBlock()->getIndex();
            for (stringList::const_iterator it = indices.begin(); it != indices.end(); ++it) {
                string potentialIndexFile = joinPaths(_reroutedPath, *it);
                try {
                    std::cout << "Serving index file: " << potentialIndexFile << std::endl;
                    this->initHttpResponse(readFileContent(potentialIndexFile), getContentType(potentialIndexFile), 200);
                    return ;
                } 
                
                catch (const HttpException& e) {
                    std::cout << "Error serving index file: " << e.getStatusCode() << " " << e.what() << std::endl;
                }
            }
            
            // if autoindex is enabled, serve the autoindex page
            if (getBlock()->getAutoindex()) {
                std::cout << "Generating auto index... " << _reroutedPath<< std::endl;
                string autoIndex = createAutoIndexHtml(_reroutedPath, _path);
                this->initHttpResponse(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            // if autoindex is disabled, return 403
            else {
                std::cout << "403" << "" << std::endl;
                throw HttpException(403);
            }
        }
        
        // if the path is a file
        else {
            string content = readFileContent((_reroutedPath));

            this->initHttpResponse(content, getContentType(_reroutedPath), 200);
        }
    } 

    catch (const HttpException& e) {
        LogStream::error() << "error: " << e.getStatusCode() << " " << e.what() << std::endl;
        this->initErrorHttpResponse(e.getStatusCode());
    }
}


void HttpResponse::handlePOST() {
    if (this->getBlock()->getCgiPass().empty()) {
        this->initErrorHttpResponse(500);
        return;
    }
    string cgiPass = this->getBlock()->getCgiPass();
    this->initCGIResponse(cgiPass, request);
}


void HttpResponse::handleDELETE() {

    if (!isPathExist(_reroutedPath)) {
        this->initErrorHttpResponse(404);
        std::cout << "FILE DOESN'T EXIST: " << _reroutedPath << std::endl;
        return; 
    } 

    if (isDirectory(_reroutedPath)) {
        this->initErrorHttpResponse(403);
    }

    int status = remove(_reroutedPath.c_str());

    if (status == 0) {
        this->initHttpResponse("File deleted successfully", CONTENT_TYPE_HTML, 200);
    } 
    
    else {
        std::cout << _reroutedPath << " could not be deleted. Status: " << status << std::endl;
        this->initErrorHttpResponse(500);
    }
}



/* CONSTRUCTOR */

HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) 
    : request(request) {
    LogStream::pending() << "Constructing response" << std::endl; 

    this->_emptyBlock        = new LocationBlock();
    string path             = request.headerGet("path");
    this->_serverBlockRef   = serverBlock;
    this->_locationBlockRef = resolveLocationBlock(path, serverBlock);



    /* RESOLVE LOCATION BLOCK */

    if (this->_locationBlockRef == NULL) {
        this->_locationBlockRef = serverBlock;
        this->_isLocation        = false;
        LogStream::success() << "Does not match any location. Defaulting" << std::endl; 
        printBorderedBox(_locationBlockRef->getInfo(), "Using block");
    }
    else {
        this->_isLocation = true;
        LogStream::success() << "Location matches location block: " << this->getBlock()->getUri() << std::endl;
        printBorderedBox(this->getBlock()->getInfo(), "Using block");
    }
    

    
    /* LIMIT EXCEPT */

    stringList limitExcept = this->_locationBlockRef->getLimitExcept();

    // Check if the request method is allowed
    if (std::find(limitExcept.begin(), limitExcept.end(), this->request.getMethod()) == limitExcept.end()) {
        this->initErrorHttpResponse(405);
        return;
    }

    string method = this->request.getMethod();


    
    /* REDIRECTION */

    std::pair<int, string> redirectInfo = this->getBlock()->getReturn();
    if (!redirectInfo.second.empty()) {
        LogStream::pending() << "redirecting to :" << redirectInfo.second << std::endl;
        this->initRedirectResponse(redirectInfo.second, redirectInfo.first);
        return;
    }

    if (!isPathExist(this->getBlock()->getRoot()))
        this->initErrorHttpResponse(404);

    _path         = request.headerGet("path");
    _path         = urlDecode(_path);
    if (_path == applyAlias(_path))
        _aliasApplied = false;
    else
        _aliasApplied = true;
    _path         = applyAlias(_path);
    

    if (_aliasApplied)
        _reroutedPath = _path;
    else {
        _reroutedPath = reroutePath(_path);
    }

    LogStream::log() << LogStream::log().setBordered(true) 
                     << "Original path: " << request.headerGet("path") << "\n"
                     << "Alias path: "    << _path << "\n"
                     << "Rerouted path: " << _reroutedPath << std::endl;

    // if (!isPathExist(_reroutedPath))
    //     throw HttpException(404, _reroutedPath);

    /* HANDLE METHODS */

    if (method == "GET") {
        LogStream::pending() << "Handling GET" << std::endl;
        this->handleGET(serverBlock);
    }

    else if (method == "POST") {
        LogStream::pending() << "Handling POST" << std::endl;
        this->handlePOST();
    }

    else if (method == "DELETE") {
        LogStream::pending() << "Handling DELETE" << std::endl;
        this->handleDELETE();
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

    if (this->_isLocation)
        reroutedPath = joinPaths(this->getBlock()->getRoot(), urlPath);

    return reroutedPath;
}




string HttpResponse::createAutoIndexHtml(string path, string root) {

    // std::cout << "PATH:: " << path << std::endl;
    // std::cout << "ROOT:: " << root << std::endl;

    std::stringstream output;
    output << "<html><head><title>Index of " << path << "</title>"
           << "<style>" << HttpResponse::css << "</style>"
           << "<script>"
           << "async function deleteFile(filePath, rowId) {"
           << "  if (!confirm('Delete this file?')) return;"
           << "  try {"
           << "    const response = await fetch(filePath, { method: 'DELETE' });"
           << "    if (response.ok) {"
           << "      document.getElementById(rowId).remove();"
           << "    } else {"
           << "      alert('Failed to delete: ' + response.status);"
           << "    }"
           << "  } catch (err) {"
           << "    alert('Error: ' + err);"
           << "  }"
           << "}"
           << "</script>"
           << "</head><body>";
    
    stringList breadcrumbList = splitString(root, '/');
    std::stringstream breadcrumbHtml;
    breadcrumbHtml << "<a href=\"" << "/" << "\">" << "~" << "</a>";
    for (stringList::iterator it = breadcrumbList.begin(); it != breadcrumbList.end(); it++) {
        std::stringstream ss;
        for (stringList::iterator it2 = breadcrumbList.begin(); it2 != (it + 1); it2++)
            ss << *it2 << ((it2 != it) ? "/" : "");
        breadcrumbHtml << "<a href=\"" << ss.str() << "\">" << *it << "</a>";
        breadcrumbHtml << ((it + 1 != breadcrumbList.end()) ? " / " : " ");
    }

    output << "<h1>Index of " << breadcrumbHtml.str() << "</h1><table>";
    output << "<tr><th>Name</th><th>Size</th><th>Action</th></tr>";

    stringList files = listFiles(path);
    if (!files.empty()) {
        int rowCounter = 0;
        for (stringList::iterator it = files.begin(); it != files.end(); ++it) {
            if (*it == "." || *it == "..")
                continue;

            string rowId = "row" + to_string(rowCounter++);
            string fullPath = joinPaths(path, *it);
            string relativePath = joinPaths(root, *it); // Use relative path for DELETE
            string fileSize = isDirectory(fullPath) ? "-" : to_string(getFileSize(fullPath));
            string hrefPath = relativePath; // Full URL for display

            output << "<tr id=\"" << rowId << "\"><td>";

            if (isDirectory(fullPath))
                output << "<a class=\"folder\" href=\"" << hrefPath << "\">" << *it << "</a>";
            else
                output << "<a href=\"" << hrefPath << "\">" << *it << "</a>";

            output << "</td><td>" << fileSize << " bytes</td>";

            if (!isDirectory(fullPath))
                output << "<td><button onclick=\"deleteFile('" << hrefPath << "', '" << rowId << "')\">Delete</button></td>";
            else
                output << "<td></td>";

            output << "</tr>";
        }
    } 
    
    else {
        output << "<tr><td colspan=\"3\">Unable to open directory</td></tr>";
    }

    output << "</table></body></html>";
    return output.str();
}




/* GETTERS */

string HttpResponse::getFinalResponseMsg() const { return _finalResponseMsg; }
string HttpResponse::getMethod()           const { return _method; }
int    HttpResponse::getContentLength()    const { return _contentLength; }




string HttpResponse::composeHttpResponse(
    const string& body,
    int           statusCode,
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
    this->_rawContent       = body;
    this->_contentType      = resourceType;
    this->_statusCode       = statusCode;
    this->_message          = CodeToMessage(statusCode).second;
    // this->finalResponseMsg = composeResponse(body, resourceType, to_string(statusCode), message);
    this->_finalResponseMsg = composeHttpResponse(body, statusCode, _message, 
                                "Content-Length",  to_string(body.size()).c_str(),
                                "Connection",      "close",
                                "Content-Type",    resourceType.c_str(),
                                NULL);
}



void HttpResponse::initRedirectResponse(string &redirectUrl, int statusCode) {

    this->_rawContent    = "";
    this->_contentType   = CONTENT_TYPE_HTML;
    this->_statusCode    = statusCode;
    this->_message       = CodeToMessage(statusCode).second;

    this->_finalResponseMsg = composeHttpResponse("", statusCode, _message,
        "Location",        redirectUrl.c_str(),
        "Content-Length",  "0",
        "Content-Type",    CONTENT_TYPE_HTML,
        "Cache-Control",   "no-cache; no-store; must-revalidate",
        "Pragma",          "no-cache",
        "Connection",      "close",
        NULL);
}

void HttpResponse::initErrorHttpResponse(int statusCode, string error, string description) {
    std::map<int, string> errorPages = this->getBlock()->getErrorPage();

    if (!errorPages.empty() && errorPages.find(statusCode) != errorPages.end()) {
        std::cout << "REROUTED PATH" << _reroutedPath << std::endl;
        string basepath = this->getBlock()->getRoot();
        if (_aliasApplied)
            basepath = this->getBlock()->getAlias();

        if (basepath[0] == '/')
            basepath = basepath.substr(1);
        string errorPage = joinPaths(basepath, errorPages.find(statusCode)->second);
        

        try {
            this->initHttpResponse(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
            return ;
        }
        catch (const HttpException& e) { 
            LogStream::info() << "Cannot find " << errorPages[statusCode] << " in " << errorPage << ". Fallback to default error page" << std::endl;
        }
    }

    std::pair<string, string> details = CodeToMessage(statusCode);
    
    string errorMsg         = error.empty() ? details.first : error;
    string errorDescription = description.empty() ? details.second : description;
    
    std::stringstream output;
    output  << "<html><head><title>" << errorMsg << "</title>"
            << "<style>" << HttpResponse::css << "</style></head><body>"
            << "<h1 class=\"error\">" << errorMsg << "</h1>"
            << "<p>" << errorDescription << "</p>"
            << "</body></html>";
    this->initHttpResponse(output.str(), CONTENT_TYPE_HTML, statusCode);
}


void HttpResponse::initCGIResponse(string cgiPath, HttpRequest request) {

    CGIHandler cgiHandler = CGIHandler();
    LogStream::pending() << "Initializing CGI: \"" << cgiPath << "\"" << std::endl;

    if (!isPathExist(makeAbsPath(cgiPath))) {
        LogStream::error() << "path doesn't exist: " << cgiPath << std::endl;
        this->initErrorHttpResponse(500);
        return;
    }

    string pathToHandle = request.headerGet("path_info");
    int exit_status;
    this->_finalResponseMsg = cgiHandler.handleCgi(cgiPath, request, exit_status, *this->getBlock(), *this);
}




/* OTHERS */

string HttpResponse::getContentType(const string& resourcePath) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        if (endsWith(resourcePath, it->first))
            return it->second;
    }
    
    return "text/plain";
}

string HttpResponse::_dirHasIndexFile(string path) {
    const std::vector<string>& indices = _serverBlockRef->getIndex();
    
    for (std::vector<string>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
        if (isPathExist(path + "/" + *it))
            return path + "/" + *it;
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


string HttpResponse::getReroutedPath() {
    return _reroutedPath;
}


string HttpResponse::applyAlias(string& path) {
    if (!this->_isLocation) {
        return path;
    }

    string alias = this->getBlock()->getAlias();
    string uri = this->getBlock()->getUri();

    if (!alias.empty() && startsWith(path, uri)) {
        string suffix = path.length() > uri.length() ? path.substr(uri.length()) : "";
        string newPath = alias;

        // Ensure we don't end up with double slashes or missing slashes
        if (!alias.empty() && alias[alias.size() - 1] != '/' && !suffix.empty() && suffix[0] != '/')
            newPath += '/';

        newPath += suffix;

        // Check if it's a directory
        if (isPathExist(newPath) && isDirectory(newPath)) {
            return newPath;
        }

        return newPath;
    }

    return path;
}

std::pair<string, string> HttpResponse::GetMsg(int statusCode) const {
    switch (statusCode) {
        case 200: return std::make_pair("OK", "Success");
        case 201: return std::make_pair("Created", "The request has been fulfilled and resulted in a new resource being created.");
        case 202: return std::make_pair("Accepted", "The request has been accepted for processing, but the processing has not been completed.");
        case 302: return std::make_pair("Found", "The requested resource resides temporarily under a different URI.");
        case 303: return std::make_pair("See Other", "The server is redirecting the client to a different resource, typically using a GET request.");
        case 400: return std::make_pair("Bad Request", "The server could not understand the request due to invalid syntax.");
        case 401: return std::make_pair("Unauthorized", "The client must authenticate itself to get the requested response.");
        case 405: return std::make_pair("Method Not Allowed", "The request method is known by the server but has been disabled and cannot be used.");
        case 403: return std::make_pair("Forbidden", "The client does not have access rights to the content");
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

