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

string reroutePath(const string &urlPath) {
    stringDict pathMap;

    pathMap["cgi-bin"] = "";

    string reroutedPath = string(ROOT_RESOURCE_PATH) + "/" + urlPath;
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

string shouldPageOverride(int statusCode) {
    std::map<int, string> pageOverride;
    pageOverride[404] = "error2.html";
    //pageOverride[403] = "error2.html";
    pageOverride[405] = "error2.html";
    
    if (pageOverride.find(statusCode) != pageOverride.end()) {
        return pageOverride[statusCode];
    }
    return "error.html";
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
    string absolutePath = getAbsolutePath(reroutePath(fileToHandle));

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

bool containsIndexFile(string path) {
    return doesPathExist(path + "/index.html");
}

HttpResponse::HttpResponse(HttpRequest &request, ServerBlock *ServerBlock) {
    this->_serverBlockRef = ServerBlock;

    string path = request.getParam("path");

    std::vector<std::string> allowedMethods = ServerBlock->getLimitExcept();

    try {
        for (int i = 0; i < allowedMethods.size(); i++) {
            if (request.getMethod() == allowedMethods[i]) {
                std::cout << GREEN << "Method is supported" << RESET << std::endl;
                break;
            }
            if (i == allowedMethods.size() - 1) {
                throw HttpException(405);
            }
        }

        string reroutedPath = reroutePath(path);
        string content = readFileContent(reroutedPath);

        if (isDirectory(reroutedPath)) {
            if (containsIndexFile(reroutedPath)) {
                try {
                    this->setHttpResponseSelf(readFileContent(reroutedPath + "/index.html"), CONTENT_TYPE_HTML, 200);
                }
                catch (const HttpException& e) {
                    std::cout<< RED << "invalid access" << RESET << std::endl;
                }
            }
            
            if (AUTO_INDEX_ON) {
            std::cout << GREEN << "Auto index is on" << RESET << std::endl;
                string autoIndex = generateAutoIndexHtml(reroutedPath);
                this->setHttpResponseSelf(autoIndex, CONTENT_TYPE_HTML, 200);
            } 
            
            else {
                throw HttpException(403);
            }
        }

        // Check if CGI handling is required for the resource
        string cgiToUse = decideCGIToUse(path);

        // If CGI handling is required, reroute the path to the CGI script
        if (!cgiToUse.empty()) {
            this->callCGIResponse(cgiToUse, path, request);
        }
        else if (isCGI(path)) {
            this->callCGIResponse(path, request.getParam("path_info"), request);
        }
        else {
            this->setHttpResponseSelf(content, getContentType(reroutedPath), 200);
        }

        std::cout << this->getFinalResponseMsg() << std::endl;
    } 
    

    catch (const HttpException& e) {
        std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
        //return createErrorHTTPResponse(e.getStatusCode());
    }
}

// HttpResponse::HttpResponse(HttpRequest &request, ServerBlock &ServerBlock) {
//     string path = request.getParam("path");

//     std::vector<std::string> allowedMethods;
//     allowedMethods.push_back("GET");
//     allowedMethods.push_back("POST");
//     allowedMethods.push_back("DELETE");
    

//     try {
//         for (int i = 0; i < allowedMethods.size(); i++) {
//             if (request.getMethod() == allowedMethods[i]) {
//                 std::cout << GREEN << "Method is supported" << RESET << std::endl;
//                 break;
//             }
//             if (i == allowedMethods.size() - 1) {
//                 throw HttpException(405);
//             }
//         }

//         string reroutedPath = reroutePath(path);
//         string content = readFileContent(reroutedPath);

//         if (isDirectory(reroutedPath)) {
//             if (containsIndexFile(reroutedPath)) {
//                 try {
//                     return HttpResponse(readFileContent(reroutedPath + "/index.html"), CONTENT_TYPE_HTML, 200);
//                 }
//                 catch (const HttpException& e) {
//                     std::cout<< RED << "invalid access" << RESET << std::endl;
//                 }
//             }
            
//             if (AUTO_INDEX_ON) {
//             std::cout << GREEN << "Auto index is on" << RESET << std::endl;
//                 string autoIndex = generateAutoIndexHtml(reroutedPath);
//                 return HttpResponse(autoIndex, CONTENT_TYPE_HTML, 200);
//             } 
            
//             else {
//                 throw HttpException(403);
//             }
//         }

//         // Check if CGI handling is required for the resource
//         string cgiToUse = decideCGIToUse(path);

//         // If CGI handling is required, reroute the path to the CGI script
//         if (!cgiToUse.empty()) {
//             return callCGIResponse(cgiToUse, path, request);
//         }
//         else if (isCGI(path)) {
//             return callCGIResponse(path, request.getParam("path_info"), request);
//         }
//         else
//             return HttpResponse(content, getContentType(reroutedPath), 200);

//     } 
    
//     catch (const HttpException& e) {
//         std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
//         return createErrorHTTPResponse(e.getStatusCode());
//     }
// }

// HttpResponse HttpResponse::createHttpResponse(HttpRequest &request) {
//     string path = request.getParam("path");

//     std::vector<std::string> allowedMethods;
//     allowedMethods.push_back("GET");
//     allowedMethods.push_back("POST");
//     allowedMethods.push_back("DELETE");
    

//     try {
//         for (int i = 0; i < allowedMethods.size(); i++) {
//             if (request.getMethod() == allowedMethods[i]) {
//                 std::cout << GREEN << "Method is supported" << RESET << std::endl;
//                 break;
//             }
//             if (i == allowedMethods.size() - 1) {
//                 throw HttpException(405);
//             }
//         }

//         string reroutedPath = reroutePath(path);
//         string content = readFileContent(reroutedPath);

//         if (isDirectory(reroutedPath)) {
//             if (containsIndexFile(reroutedPath)) {
//                 try {
//                     return HttpResponse(readFileContent(reroutedPath + "/index.html"), CONTENT_TYPE_HTML, 200);
//                 }
//                 catch (const HttpException& e) {
//                     std::cout<< RED << "invalid access" << RESET << std::endl;
//                 }
//             }
            
//             if (AUTO_INDEX_ON) {
//             std::cout << GREEN << "Auto index is on" << RESET << std::endl;
//                 string autoIndex = generateAutoIndexHtml(reroutedPath);
//                 return HttpResponse(autoIndex, CONTENT_TYPE_HTML, 200);
//             } 
            
//             else {
//                 throw HttpException(403);
//             }
//         }

//         // Check if CGI handling is required for the resource
//         string cgiToUse = decideCGIToUse(path);

//         // If CGI handling is required, reroute the path to the CGI script
//         if (!cgiToUse.empty()) {
//             return callCGIResponse(cgiToUse, path, request);
//         }
//         else if (isCGI(path)) {
//             return callCGIResponse(path, request.getParam("path_info"), request);
//         }
//         else
//             return HttpResponse(content, getContentType(reroutedPath), 200);

//     } 
    
//     catch (const HttpException& e) {
//         std::cout << RED << "error: " << e.what() << RESET << e.getStatusCode() << std::endl;
//         return createErrorHTTPResponse(e.getStatusCode());
//     }
// }



/*
note: the program should handle hte case in which the cgi contains content length in its header

The CGI's output, if it provides a Content-Length, would look like this:

Content-Length: 48  <-- Important: newline after 48!
                   <-- Blank line
<h1>Hello from CGI!</h1>
<p>This is some dynamic content.</p>


how should the POST method be handled? 


*/