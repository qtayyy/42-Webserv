#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "Webserv.hpp"
#include <iostream>
#include <string>

class LoggerBuffer : public std::streambuf {
    public:
        LoggerBuffer();
        static std::string setColor(string color);

    protected:
        virtual int overflow(int c);

    private:
        static std::ostream& out_stream;
        static std::string current_color;
};

class Logger : public std::ostream {
    public:
        Logger();

    private:
        LoggerBuffer buffer;
};

#endif