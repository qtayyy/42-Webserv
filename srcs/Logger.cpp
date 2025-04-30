#include "Logger.hpp"

std::string LoggerBuffer::current_color = "";
std::ostream* LoggerBuffer::out_stream = &std::cout;
std::ofstream LoggerBuffer::file_stream;


bool LoggerBuffer::isColorCode(const std::string& str) {
    static const std::string colorCodes[] = {
        BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, GRAY, LIGHT_GRAY, BEIGE, RESET
    };
    for (size_t i = 0; i < sizeof(colorCodes) / sizeof(colorCodes[0]); ++i) {
        if (colorCodes[i] == str) {
            return true;
        }
    }
    return false;
}


int LoggerBuffer::overflow(int c) {
    static std::string buffer; // Temporary buffer to accumulate characters for comparison

    if (c != EOF) {
        buffer += static_cast<char>(c);

        if (isColorCode(buffer)) {
            if (out_stream != &std::cout) {
                // If outputting to a file, skip the color code
                buffer.clear();
                return c;
            }
        }

        // if (buffer.size() > 0 && !isColorCode(buffer)) {
            for (std::string::iterator it = buffer.begin(); it != buffer.end(); ++it) {
                (*out_stream).put(*it);
            }
            buffer.clear();
        // }

        if (c == '\n') {
            if (out_stream == &std::cout) {
                (*out_stream) << RESET; // Reset color for console
            }
            LoggerBuffer::current_color = ""; // Clear the color
            // LoggerBuffer::out_stream = &std::cout; // Reset to default stream
        }
    }
    return c;
}

const int Logger::RESET_COLOR = 1 << 0; 
const int Logger::RESET_STREAM = 1 << 1;
const int Logger::RESET_ALL = Logger::RESET_COLOR | Logger::RESET_STREAM;

int LoggerBuffer::reset(int flags) {
    if (flags & Logger::RESET_COLOR) {
        LoggerBuffer::current_color = ""; // Clear the color
    }
    if (flags & Logger::RESET_STREAM) {
        LoggerBuffer::out_stream = &std::cout; // Reset to default stream
    }
    return 0;
}

string Logger::reset() {
    LoggerBuffer::reset(Logger::RESET_ALL);
    return "\n";
}

LoggerBuffer::LoggerBuffer() {}
Logger::Logger() : std::ostream(&buffer), buffer() {}

// Set the stream to an existing stream (e.g., std::cerr)
string Logger::setStream(std::ostream* stream) {
    LoggerBuffer::out_stream = stream;
    return "";
}

// Set the stream to a file stream
string Logger::setStream(const std::string& filename, std::ios_base::openmode mode) {
    if (LoggerBuffer::file_stream.is_open()) {
        LoggerBuffer::file_stream.close();
    }
    LoggerBuffer::file_stream.open(filename.c_str(), mode);
    if (LoggerBuffer::file_stream.is_open()) {
        LoggerBuffer::out_stream = &LoggerBuffer::file_stream;
    } 
    
    else {
        LoggerBuffer::out_stream = &std::cerr; // Fallback to std::cerr if file fails to open
        (*LoggerBuffer::out_stream) << "Failed to open file: " << filename << std::endl;
    }
    return "";
}

std::string Logger::setColor(string color) {
    LoggerBuffer::current_color = color;
    return "";
}

