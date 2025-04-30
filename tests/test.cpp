#include <iostream>
#include <string>
#include <map>
#include <cstdarg>
#include <sstream>

std::string initHttpResponse(
    const std::string& body,
    int statusCode = 200,
    const std::string& statusMessage = "OK",
    ... // key, value, key, value, ..., NULL
) {
    std::map<std::string, std::string> headers;
    va_list args;
    va_start(args, statusMessage);

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
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n" << body;

    return response.str();
}

int main() {
    std::string respone = initHttpResponse("hello world", 200, "hello", 
        "Content-length", "500",
        "test", "400", NULL);
    std::cout << respone << std::endl;
}