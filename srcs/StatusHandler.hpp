#ifndef STATUSHANDLER_HPP
#define STATUSHANDLER_HPP

#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include "Utils.hpp"


class StatusHandler {
private:
    int httpStatusCode;       // HTTP status code (e.g., 404, 500)
    string errorMessage; // Short error message
    string details;      // Detailed explanation
    string requestUrl;   // URL that caused the error
    string method;       // HTTP method (GET, POST, etc.)
    std::time_t timestamp;    // Error occurrence time

public:
    StatusHandler(
            int code, 
            const string& message, 
            const string& detail,
            const string& url, 
            const string& reqMethod
        );

    StatusHandler(int code);

    int getStatusCode() const;
    string getErrorMsg() const;
    string getDetails() const;
    string getRequestUrl() const;
    string getMethod() const;
    string getTimestamp() const;

    void displayError() const;

    static string CodeToMessage(int code);

    string generateErrorPage(string errorPagePath) const;
};


#endif