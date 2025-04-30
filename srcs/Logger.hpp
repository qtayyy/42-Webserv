#ifndef LOGGER_HPP
#define LOGGER_HPP
#include "Webserv.hpp"

class LoggerBuffer : public std::streambuf {
    public:
        static int reset(int flags);
        LoggerBuffer();
        static std::ostream* out_stream;
        static std::ofstream file_stream; // File stream for dynamic file output
        static std::string current_color;

    protected:
        bool isColorCode(const std::string &str);
        virtual int overflow(int c);

    private:
};

class Logger : public std::ostream {
    public:
    Logger();
        static const int RESET_COLOR; 
        static const int RESET_STREAM;
        static const int RESET_ALL;
        static string reset();

        static string setStream(std::ostream* stream);
        static string setStream(const std::string& filename, std::ios_base::openmode mode = std::ios::out);

        static std::string setColor(string color);


    private:
        LoggerBuffer buffer;
};

#endif