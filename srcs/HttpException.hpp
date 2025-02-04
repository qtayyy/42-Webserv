#ifndef HTTP_EXCEPTION_HPP
#define HTTP_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include "StatusHandler.hpp"

class HttpException : public std::runtime_error {
public:
    HttpException(int statusCode) : std::runtime_error("test"), statusCode_(statusCode) {}

    int getStatusCode() const {
        return statusCode_;
    }

private:
    int statusCode_;
};

#endif // HTTP_EXCEPTION_HPP