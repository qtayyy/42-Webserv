#ifndef LOG_HPP
#define LOG_HPP


#include "Webserv.hpp"
#include "Utils.hpp"

class LogStream {
    protected:
        std::ostringstream buffer;
        std::ostream       *out; // Pointer to the output stream
        const char         *colorCode;
        std::ofstream       fileStream; // File stream for file output
        std::ostream       *defaultStream; // Default stream (e.g., std::cout or std::cerr)
        string             ending;
        bool               bordered;
        bool               supress;
        std::ostream       *borderedStream;
    
        LogStream(std::ostream& os, const char* color, string const &ending="")
            : out(&os), colorCode(color), defaultStream(&os), ending(ending), bordered(false), borderedStream(&os) {}
    
        void flush() {
            if (bordered) {
                printBorderedBox(buffer.str(), ending);
            }

            else if (out && !supress) {
                *out << colorCode << buffer.str();
                if (out == &std::cout || out == &std::cerr) {
                    *out << "\033[0m";
                    *out << ending;
                }
                *out << std::endl;
            }
            buffer.str("");
            buffer.clear();
            bordered = false;
        }
    
    public:
        ~LogStream() {
            fileStream.close(); 
        }
    
        LogStream() : out(&std::cout), colorCode("\033[0m"), defaultStream(&std::cout) {}
    
        LogStream(const string& filename, const char* color = "\033[0m") 
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
    
        string toggleSupression(bool supress) {
            this->supress = supress;
            return "";
        }

        string setBordered(bool bordered) {
            this->bordered = bordered;
            this->out = bordered ? borderedStream : defaultStream;
            return "";
        }

        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
                flush();
            else
                buffer << manip;
            return *this;
        }
    


        LogStream &setStream(std::ostream& os) {
            this->fileStream.close();
            this->out = &os;
            return *this;
        }

        LogStream &setstream(const string& filepath, std::ios_base::openmode mode) {
            this->fileStream.close();

            this->fileStream.open(filepath.c_str(), mode);
            if (this->fileStream.is_open())
                this->out = &this->fileStream;
            else {
                supress = true;
                this->out = &std::cerr;
            }
            return *this;
        }
    
        LogStream &resetstream() {
            fileStream.close();
            out = defaultStream;
            return *this;
        }
    
        typedef std::ios_base::openmode openmode;
    
        static LogStream& log(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cout, ""); // default to cout with no color
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }
        
        static LogStream& info(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cout, BLUE); // Blue
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }

        static LogStream& pending(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cout, YELLOW, "..."); // Beige
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }
        
        static LogStream& warning(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cout, MAGENTA); // Yellow
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }
        
        static LogStream& success(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cout, GREEN); // Yellow
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }

        static LogStream& error(string outputFile = "", openmode mode = std::ios::out) {
            static LogStream instance(std::cerr, RED); // Red
            return outputFile.empty() ? instance.resetstream() : instance.setstream(outputFile, mode);
        }
        
    };

#endif