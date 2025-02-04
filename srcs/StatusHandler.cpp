#include "StatusHandler.hpp"
#include "Utils.hpp"

/**
 @brief Constructor for StatusHandler class
*/
StatusHandler::StatusHandler(
        int code, 
        const string& message, 
        const string& detail,
        const string& url, 
        const string& reqMethod
    ):
    httpStatusCode(code), 
    errorMessage(message), 
    details(detail),
    requestUrl(url),
    method(reqMethod) {
    timestamp = std::time(NULL); // Set current time
}

StatusHandler::StatusHandler(int code) {
    httpStatusCode = code;

    std::vector<string> parts = splitBy1stInstance(CodeToMessage(code), ';');
    this->errorMessage = parts[0];
    this->details = parts[1];
    this->requestUrl = "No URL available.";
    this->method = "No method available.";
    this->timestamp = std::time(NULL);
}

int StatusHandler::getStatusCode()           const { return httpStatusCode; }
string StatusHandler::getErrorMsg()     const { return errorMessage; }
string StatusHandler::getDetails()      const { return details; }
string StatusHandler::getRequestUrl()   const { return requestUrl; }
string StatusHandler::getMethod()       const { return method; }
string StatusHandler::getTimestamp()    const {
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
    return string(buffer);
}

void StatusHandler::displayError() const {
    std::cout << "HTTP " << httpStatusCode << " - " << errorMessage << "\n"
              << "Details: " << details << "\n"
              << "Request URL: " << requestUrl << "\n"
              << "Method: " << method << "\n"
              << "Timestamp: " << getTimestamp() << "\n";
}

string StatusHandler::CodeToMessage(int code) {
    switch (code) {
        
        case 200: return "OK;The request has succeeded.";
        case 201: return "Created;The request has been fulfilled and resulted in a new resource being created.";
        case 400: return "Bad Request;The server could not understand the request due to invalid syntax.";
        case 401: return "Unauthorized;The client must authenticate itself to get the requested response.";
        case 405: return "Method Not Allowed;The request method is known by the server but has been disabled and cannot be used.";
        case 403: return "Forbidden;The client does not have access rights to the content, i.e., they are unauthorized.";
        case 404: return "Not Found;The server can not find the requested resource.";
        case 500: return "Internal Server Error;Something went wrong on the server.";
        case 504: return "Gateway Timeout;The server, while acting as a gateway or proxy, did not receive a timely response from an upstream server it needed to access in order to complete the request.";
    }
    return "Unknown error code.";
}

string StatusHandler::generateErrorPage(string errorPagePath) const {
    string errorFileContents = readFileContent(errorPagePath);
    
    replaceIfFound(&errorFileContents, "{{ERROR_CODE}}", to_string(httpStatusCode));
    replaceIfFound(&errorFileContents, "{{ERROR_MESSAGE}}", errorMessage);
    replaceIfFound(&errorFileContents, "{{DETAILS}}", details);
    
    return errorFileContents;
}

