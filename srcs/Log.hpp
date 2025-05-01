#ifndef LOG_HPP
#define LOG_HPP

#include "Webserv.hpp"

class LogStream {
    protected:
        std::ostringstream buffer;
        std::ostream* out; // Pointer to the output stream
        const char* colorCode;
        std::ofstream fileStream; // File stream for file output
        std::ostream* defaultStream; // Default stream (e.g., std::cout or std::cerr)
    
        LogStream(std::ostream& os, const char* color)
            : out(&os), colorCode(color), defaultStream(&os) {}
    
        void flush() {
            if (out) {
                *out << colorCode << buffer.str();
                if (out == &std::cout || out == &std::cerr) {
                    *out << "\033[0m";
                }
                *out << std::endl;
            }
            buffer.str("");
            buffer.clear();
        }
    
    public:
        ~LogStream() {
            fileStream.close(); 
        }
    
        LogStream() : out(&std::cout), colorCode("\033[0m"), defaultStream(&std::cout) {}
    
        LogStream(const std::string& filename, const char* color = "\033[0m") 
            : colorCode(color) {
            fileStream.open(filename.c_str());
            if (fileStream.is_open()) {
                out = &fileStream;
                defaultStream = &fileStream;
            } else {
                out = &std::cerr;
                defaultStream = &std::cerr;
            }
        }
    
        template<typename T>
        LogStream& operator<<(const T& val) {
            buffer << val;
            return *this;
        }
    
        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
                flush();
            } 
            else {
                buffer << manip;
            }
            return *this;
        }
    
        static int setstream(LogStream& logger, const std::string& filepath, std::ios_base::openmode mode) {
            logger.fileStream.close();
            logger.fileStream.open(filepath.c_str(), mode);
            if (logger.fileStream.is_open()) {
                logger.out = &logger.fileStream;
                return 0;
            }
            else {
                logger.out = &std::cerr;
                return -1;
            }
        }
    
        void resetstream() {
            fileStream.close();
            out = defaultStream;
        }
    
        typedef std::ios_base::openmode openmode;
    
        static LogStream& info(std::string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile.c_str(), mode), "\033[34m");
            return instance;
        }
    
        static LogStream& warning(std::string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile.c_str(), mode), "\033[33m");
            return instance;
        }
    
        static LogStream& log(std::string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile.c_str(), mode), "");
            return instance;
        }
    
        static LogStream& error(std::string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(outputFile.empty() ? std::cerr : *new std::ofstream(outputFile.c_str(), mode), "\033[31m");
            return instance;
        }
    };

#endif