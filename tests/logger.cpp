#include <iostream>
#include <sstream>
#include <fstream>
#define DEFINE_LOGGER_CLASS(className, stream, colorCode)

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
        if (out)
            *out << colorCode << buffer.str();
            if (out == &std::cout || out == &std::cerr) {
                *out << "\033[0m";
            }
            *out << std::endl;
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
        fileStream.open(filename);
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
        logger.fileStream.open(filepath, mode);
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
        static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile, mode), "\033[34m");
        return instance;
    }

    static LogStream& warning(std::string outputFile = "", openmode mode = std::ios::out) {
        static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile, mode), "\033[33m");
        return instance;
    }

    static LogStream& log(std::string outputFile = "", openmode mode = std::ios::out) {
        static LogStream instance(outputFile.empty() ? std::cout : *new std::ofstream(outputFile, mode), "");
        return instance;
    }

    static LogStream& error(std::string outputFile = "", openmode mode = std::ios::out) {
        static LogStream instance(outputFile.empty() ? std::cerr : *new std::ofstream(outputFile, mode), "\033[31m");
        return instance;
    }
};


// class Linfo : public LogStream {
//     private:
//         Linfo() : LogStream(std::cout, "\033[32m") {} // Green
//         Linfo(const Linfo&);
//         Linfo& operator=(const Linfo&);

//     public:
//         static Linfo& log(std::string const outputStream="") {
//             static Linfo instance;
//             return instance;
//         }
// };


// class Lwarning : public LogStream {
//     private:
//         Lwarning() : LogStream(std::cout, "\033[33m") {} // Yellow
//         Lwarning(const Lwarning&);
//         Lwarning& operator=(const Lwarning&);

//     public:
//         static Lwarning& log() {
//             static Lwarning instance;
//             return instance;
//         }
// };


// class Lerror : public LogStream {
//     private:
//         Lerror() : LogStream(std::cerr, "\033[31m") {} // Red
//         Lerror(const Lerror&);
//         Lerror& operator=(const Lerror&);

//         public:
//             static Lerror& log() {
//                 static Lerror instance;
//                 return instance;
//             }
//     };


int main() {
    int value = 42;

    // Linfo::log() << "Info: Value is " << value << std::endl;
    // Lwarning::log() << "Warning: Value might be too high" << std::endl;
    // Lerror::log() << "Error: Value exceeded limit!" << std::endl;

    // // Redirect Lerror to a file
    
    // Lerror::log() << LogStream::setstream(Lerror::log(), "output.txt", std::ios::trunc) << "This error is logged to a lile." << std::endl;

    // // Reset Lerror to default stream
    // Lerror::log().resetstream();
    // Lerror::log() << "This error is back to the default stream." << std::endl;

    LogStream::log("outfile.txt") << "hello" << std::endl;
    LogStream::warning() << "hello" << std::endl;
    LogStream::info() << "hello" << std::endl;
}
