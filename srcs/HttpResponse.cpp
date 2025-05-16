/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shechong <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 14:38:32 by shechong          #+#    #+#             */
/*   Updated: 2025/05/16 14:38:40 by shechong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

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


/* GETTERS */

string HttpResponse::getFinalResponseMsg() const { return _finalResponseMsg; }
int    HttpResponse::getContentLength()    const { return _contentLength; }
string HttpResponse::getReroutedPath()  { return _reroutedPath; }
LocationBlock *HttpResponse::getBlock() {
    if (this->_usingLocationBlock) {
        return dynamic_cast<LocationBlock*>(this->_resolvedLocationBlock);
    } else {
        return &_defaultBlock;
    }
}




/* REQUEST HANDLERS */

void HttpResponse::_handleGET() {

    if (!this->getBlock()->getCgiPass().empty()) {
        this->_initCGIResponse(this->getBlock()->getCgiPass(), _request);
        return;
    }
    
    if (isDirectory(_reroutedPath)) {
        LogStream::pending() << "Directory detected. Attempting to serve index" << std::endl;
        string indexFile = _dirHasIndexFile(_reroutedPath);
        
        // if index file is found, serve it
        stringList indexFiles = getBlock()->getIndex();
        for (stringList::const_iterator indexFile = indexFiles.begin(); indexFile != indexFiles.end(); ++indexFile) {
            string potentialIndexFile = joinPaths(_reroutedPath, *indexFile);
            try {
                this->_initHttpResponse(readFileContent(potentialIndexFile), _resolveContentType(potentialIndexFile), 200);
                LogStream::success() << "Served index: " << potentialIndexFile << std::endl;
                return ;
            }
            
            catch (const HttpException& e) {
                LogStream::error() << "Error serving index file: " << e.getStatusCode() << " " << e.what() << std::endl;
            }
        }
        
        // if autoindex is enabled, serve the autoindex page
        if (getBlock()->getAutoindex()) 
            this->_initHttpResponse(generateAutoIndexHtml(_reroutedPath, _path), CONTENT_TYPE_HTML, 200);

        // if autoindex is disabled, return 404
        else 
            throw HttpException(404);
    }
    
    else
        this->_initHttpResponse(readFileContent(_reroutedPath), _resolveContentType(_reroutedPath), 200);
}

void HttpResponse::_handlePOST() {
    if (this->getBlock()->getCgiPass().empty()) {
        throw HttpException(500);
    }
    string cgiPass = this->getBlock()->getCgiPass();
    this->_initCGIResponse(cgiPass, _request);
}

void HttpResponse::_handleDELETE() {

    if (!isPathExist(_reroutedPath)) throw HttpException(404);
    if (isDirectory(_reroutedPath))  throw HttpException(403);
    
    if (remove(_reroutedPath.c_str()) == 0) this->_initHttpResponse("File deleted successfully", CONTENT_TYPE_HTML, 200);
    else                                    throw HttpException(500);
}



/* CONSTRUCTOR */

HttpResponse::~HttpResponse() {
    
}



HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *serverBlock) : 
    _usingLocationBlock(false),
    _isAlias(false),
    _isExtension(false),
    _request(request),
    _serverBlockRef(serverBlock)
    {
    this->_defaultBlock = LocationBlock(serverBlock);
    this->_defaultBlock.initDefaultLocationBlockConfig();

    LogStream::pending() << "Constructing response" << std::endl; 
    try {

        /* RESOLVE LOCATION BLOCK */

        _resolvedLocationBlock = _resolveLocationBlock(request.headerGet("path"), serverBlock);
        
        LogStream::success() << "Using location block: " << "\"" << getBlock()->getUri() << "\"" << std::endl;
        LogStream::log() << LogStream::log().setBordered(true) << getBlock()->getInfo() << std::endl;

        

        /* CONTENT LENGTH */

        string contentLengthStr = request.headerGet("Content-Length");
        _contentLength = contentLengthStr.empty() ? request.getBody().size() : std::strtod(contentLengthStr.c_str(), NULL);
        if (_contentLength > getBlock()->getClientMaxBodySize())
            throw HttpException(413);


        /* LIMIT EXCEPT */

        stringList limitExcept = getBlock()->getLimitExcept();
        string          method = request.getMethod();
        if (std::find(limitExcept.begin(), limitExcept.end(), method) == limitExcept.end())
            throw HttpException(405);


        /* REDIRECTION */

        std::pair<int, string> redirectInfo = getBlock()->getReturn();
        if (!redirectInfo.second.empty()) {
            LogStream::pending() << "redirecting to :" << redirectInfo.second << std::endl;
            _initRedirectResponse(redirectInfo.second, redirectInfo.first);
            return;
        }


        /* PATH RESOLUTION */

        if (!isPathExist(getBlock()->getRoot()))
            _initErrorResponse(404);

        _path = request.headerGet("path");
        _reroutedPath = urlDecode(_path);
        _reroutedPath = _applyAlias(_reroutedPath);
        _reroutedPath = _isAlias ? _reroutedPath : _applyRoot(_reroutedPath);
        if (!_reroutedPath.empty() && _reroutedPath[0] == '/')
            _reroutedPath = _reroutedPath.substr(1);

        LogStream::success() << "Path resolved: " << request.headerGet("path") << " --> " << _reroutedPath << (_isAlias ? " (alias)" : " (root)") << std::endl;


        /* HANDLE METHODS */
    
        if      (method == "GET")    _handleGET();
        else if (method == "POST")   _handlePOST();
        else if (method == "DELETE") _handleDELETE();
        else this->_initErrorResponse(400);
    }

    catch (const HttpException& e) {
        this->_initErrorResponse(e.getStatusCode());
    }
}



/* GENERATORS */


/**
 * @brief Generates a HTML page listing the contents of a given directory path.
 *
 * This function creates an HTML page that lists the contents of a directory
 * and its subdirectories, and clickable links to navigate to them
 *
 * @param path The absolute path of the directory to list.
 * @param root The root path relative to the server's base directory, used for
 *             generating breadcrumbs and relative paths.
 * @return A string containing the generated HTML page.
 *
 * The generated HTML includes:
 * - A breadcrumb navigation bar for easy navigation.
 * - A table listing the directory contents, including:
 *   - File or folder name (with a link to navigate).
 *   - File size (or "-" for directories).
 *   - A delete button for files (not available for directories).
 *
 * The delete functionality is implemented using JavaScript and sends an
 * HTTP DELETE request to the server. If the deletion is successful, the
 * corresponding row is removed from the table.
 */
string HttpResponse::generateAutoIndexHtml(string path, string root) {
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
    breadcrumbHtml << "<a href=\"/\">~</a>";
    for (stringList::iterator breadcrumbIt = breadcrumbList.begin(); breadcrumbIt != breadcrumbList.end(); breadcrumbIt++) {
        std::stringstream pathStream;
        for (stringList::iterator subPathIt = breadcrumbList.begin(); subPathIt != (breadcrumbIt + 1); subPathIt++)
            pathStream << *subPathIt << ((subPathIt != breadcrumbIt) ? "/" : "");
        breadcrumbHtml << "<a href=\"" << pathStream.str() << "\">" << *breadcrumbIt << "</a>";
        breadcrumbHtml << ((breadcrumbIt + 1 != breadcrumbList.end()) ? " / " : " ");
    }

    output << "<h1>Index of " << breadcrumbHtml.str() << "</h1><table>";
    output << "<tr><th>Name</th><th>Size</th><th>Action</th></tr>";

    stringList files = listFiles(path);
    if (!files.empty()) {
        int rowCounter = 0;
        for (stringList::iterator currentFile = files.begin(); currentFile != files.end(); ++currentFile) {
            if (*currentFile == "." || *currentFile == "..")
                continue;

            string rowId        = "row" + to_string(rowCounter++);
            string fullPath     = joinPaths(path, *currentFile);
            string relativePath = joinPaths(root, *currentFile); // Use relative path for DELETE
            string fileSize     = isDirectory(fullPath) ? "-" : to_string(getFileSize(fullPath));
            string hrefPath     = relativePath; // Full URL for display

            output      << "<tr id=\"" + rowId + "\">"
            /* name/uri      */ << "<td><a " << (isDirectory(fullPath) ? "class=\"folder\" " : "") << "href=\"" << hrefPath << "\">" << *currentFile << "</a></td>"
            /* bytes         */ << "<td>" << fileSize << " bytes</td>" 
            /* delete-button */ << "<td>" << (!isDirectory(fullPath) ? "<button onclick=\"deleteFile('" + hrefPath + "', '" + rowId + "')\">Delete</button>" : "") << "</td>" 
                        << "</tr>";
        }
    } 
    
    else {
        output << "<tr><td colspan=\"3\">Unable to open directory</td></tr>";
    }

    output << "</table></body></html>";
    return output.str();
}

/**
 * @brief Composes an HTTP response string.
 * 
 * Returns a complete HTTP response message, including the status line,
 * headers, and body. It accepts a variable number of key-value pairs for headers.
 * 
 * @param body The body of the HTTP response.
 * @param statusCode The HTTP status code (e.g., 200, 404).
 * @param msg The status message corresponding to the status code (e.g., "OK", "Not Found").
 * @param ... <key>, <value> representing HTTP headers. The list must be terminated with a NULL pointer.
 * 
 * @return A string containing the complete HTTP response.
 */
string HttpResponse::composeHttpResponse(
    const string& body,
    int           statusCode,
    const string msg,
    ... ) 
{
    std::map<string, string> headers;
    va_list args;
    va_start(args, msg);

    while (true) {
        const char* key = va_arg(args, const char*);
        if (!key) 
            break; // NULL signals end of pairs
        const char* value = va_arg(args, const char*);
        if (!value)
            break; // malformed input
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



/* PATH RESOLUTION */

string HttpResponse::_applyRoot(string urlPath) {
    string reroutedPath = joinPaths(this->_serverBlockRef->getRoot(), urlPath);

    if (this->_usingLocationBlock)
        reroutedPath = joinPaths(this->getBlock()->getRoot(), urlPath);

    return reroutedPath;
}

string HttpResponse::_applyAlias(string& path) {
    if (!this->_usingLocationBlock)
        return path;

    string alias = this->getBlock()->getAlias();
    string uri   = this->getBlock()->getUri();

    if (alias.empty())
        return path;

    if (_isExtension) {
        _isAlias = true;
        return joinPaths(alias, getBasename(path)); // Append only the filename
    }

    if (startsWith(path, uri)) {
        string suffix = path.substr(uri.length()); // Keep the rest after matched location
        string newPath = alias;

        if (!newPath.empty() && *newPath.rbegin() != '/' && !suffix.empty() && *suffix.begin() != '/')
            newPath += '/';

        newPath += suffix;
        _isAlias = true;
        return newPath;
    }

    return path;
}

Block* HttpResponse::_resolveLocationBlock(const string& path, ServerBlock* serverBlock) {
    std::vector<LocationBlock>* locations = serverBlock->getLocation();
    LocationBlock* locationBlock = NULL;

    // Check for extension-based matches first
    for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
        const string& uri = it->getUri();

        if (startsWith(uri, ".") && endsWith(path, uri)) {
            _isExtension = true;
            locationBlock = &(*it);
            break;
        }
    }

    // If no extension-based match is found, check for exact location matches
    if (locationBlock == NULL) {
        for (std::vector<LocationBlock>::iterator it = locations->begin(); it != locations->end(); ++it) {
            const string& uri = it->getUri();

            // if (uri == "/" && locationBlock == NULL)
            //     locationBlock = &(*it);

            if (startsWith(path, uri) &&
                (path.size() == uri.size() || path[uri.size()] == '/') &&
                (locationBlock == NULL || uri.size() > locationBlock->getUri().size()))
            {
                locationBlock = &(*it);
            }
        }
    }


    _usingLocationBlock = (locationBlock != NULL);

    return locationBlock;
}



/* INITIALIZERS */

void HttpResponse::_initHttpResponse(string body, string resourceType, int statusCode) {
    this->_contentType      = resourceType;
    this->_statusCode       = statusCode;
    this->_message          = CodeToMessage(statusCode).second;
    // this->finalResponseMsg = composeResponse(body, resourceType, to_string(statusCode), message);
    this->_finalResponseMsg = HttpResponse::composeHttpResponse(body, statusCode, _message, 
                                "Content-Length",  to_string(body.size()).c_str(),
                                "Connection",      "close",
                                "Content-Type",    resourceType.c_str(),
                                NULL);
}

void HttpResponse::_initRedirectResponse(string &redirectUrl, int statusCode) {

    this->_contentType   = CONTENT_TYPE_HTML;
    this->_statusCode    = statusCode;
    this->_message       = CodeToMessage(statusCode).second;

    this->_finalResponseMsg = HttpResponse::composeHttpResponse("", statusCode, _message,
        "Location",        redirectUrl.c_str(),
        "Content-Length",  "0",
        "Content-Type",    CONTENT_TYPE_HTML,
        "Cache-Control",   "no-cache; no-store; must-revalidate",
        "Pragma",          "no-cache",
        "Connection",      "close",
        NULL);
}

void HttpResponse::_initErrorResponse(int statusCode, string error, string description) {
    std::map<int, string> errorPages = this->getBlock()->getErrorPage();

    if (!errorPages.empty() && errorPages.find(statusCode) != errorPages.end()) {
        string basepath = this->getBlock()->getRoot();
        if (_isAlias)
            basepath = this->getBlock()->getAlias();

        if (basepath[0] == '/')
            basepath = basepath.substr(1);

        string errorPage = joinPaths(basepath, errorPages.find(statusCode)->second);

        try {
            this->_initHttpResponse(readFileContent(errorPage), CONTENT_TYPE_HTML, statusCode);
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
    output  << "<html><head><title>" << errorMsg << " (" << statusCode << ")</title>"
            << "<style>" << HttpResponse::css << "</style></head><body>"
            << "<h1 class=\"error\">" << errorMsg << " (" << statusCode << ")</h1>"
            << "<p>" << errorDescription << "</p>"
            << "</body></html>";
    this->_initHttpResponse(output.str(), CONTENT_TYPE_HTML, statusCode);
}


void HttpResponse::_initCGIResponse(string cgiPath, HttpRequest request) {

    CGIHandler cgiHandler = CGIHandler();
    LogStream::pending() << "Initializing CGI: \"" << cgiPath << "\"" << std::endl;

    if (!isPathExist(makeAbsPath(cgiPath))) {
        LogStream::error() << "path doesn't exist: " << cgiPath << std::endl;
        this->_initErrorResponse(500);
        return;
    }

    string pathToHandle = request.headerGet("path_info");
    int exit_status;
    this->_finalResponseMsg = cgiHandler.handleCgi(cgiPath, request, exit_status, *this->getBlock(), *this);
}



/* OTHERS */

string HttpResponse::_resolveContentType(const string& path) {
    
    for (stringDict::const_iterator it = contentTypeMap.begin(); it != contentTypeMap.end(); ++it) {
        if (endsWith(path, it->first))
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



/* ERROR CODE */

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
        case 413: return std::make_pair("Payload Too Large", "The request is larger than the server is willing or able to process.");
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

