#include "Logger.hpp"

string LoggerBuffer::current_color = "";

std::ostream& LoggerBuffer::out_stream = std::cout;

int LoggerBuffer::overflow(int c) {
    if (c != EOF) {
        if (LoggerBuffer::current_color != "") {
            out_stream << LoggerBuffer::current_color;
        }
        out_stream.put(c);

        if (c == '\n') { // Reset current_color after a newline
            out_stream << "\033[0m"; // Reset text current_color
            LoggerBuffer::current_color = ""; // Reset red flag
        }
    }
    return c;
}


LoggerBuffer::LoggerBuffer() {}
Logger::Logger() : std::ostream(&buffer), buffer() {}

std::string LoggerBuffer::setColor(string color) {
    LoggerBuffer::current_color = color;
    return "";
}

