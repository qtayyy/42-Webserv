#include "cgi_exec.hpp"

std::string urlDecode(const std::string &encoded) {
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%') {
            if (i + 2 < encoded.length()) {
                std::istringstream hex(encoded.substr(i + 1, 2));
                int value;
                if (hex >> std::hex >> value) {
                    decoded << static_cast<char>(value);
                    i += 2;
                }
            }
        } 
        else if (encoded[i] == '+')
            decoded << ' ';
        else 
            decoded << encoded[i];
    }
    return decoded.str();
}


std::string getContentType(const std::string& resourcePath) {
    std::string resource_type = resourcePath.substr(resourcePath.find_last_of(".") + 1);
    if      (resource_type == "html") return "text/html";
    else if (resource_type == "css")  return "text/css";
    else if (resource_type == "js")   return "text/javascript";
    else if (resource_type == "jpeg"  || resource_type == "jpg") return "image/jpg";
    else if (resource_type == "png")  return "image/png";
    else if (resource_type == "pdf")  return "application/pdf";
    else                              return "text/plain";
}


std::string constructResponseWithHeader(const std::string &body, const std::string &content_type, const std::string &status_code, const std::string &status_message) {
    std::ostringstream headers;
    headers << "HTTP/1.1 " << status_code << " " << status_message << "\n";
    headers << "Content-Type: " + content_type + "\n";
    headers << "Connection: close\n";
    headers << "Content-Length: " << body.size() << "\n\n";

    // Combine headers and file content
    std::string response = headers.str() + body;

    return response;
}


std::string constructHttpResponseFromFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    // Read the entire file content into a string
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string content_type = getContentType(path);

    return constructResponseWithHeader(file_content, content_type, "200", "OK");
}

void appendLineToFile(const std::string& path, const std::string& line) {
    std::ofstream file(path.c_str(), std::ios::app);
    if (file.is_open()) {
        file << line << std::endl;
        file.close();
    } else {
        std::cerr << "Error: Could not open file at " << path << std::endl;
    }
}


bool doesPathExist(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}


bool isDirectory(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}



void createFile(const std::string& path, const std::string& content) {
    std::ofstream file(path.c_str());
    if (file.is_open()) {
        file << content;
        file.close();
        std::cout << "File created successfully at " << path << std::endl;
    } else {
        std::cerr << "Error: Could not create file at " << path << std::endl;
    }
}


#include <sstream>
#include <vector>

std::vector<std::string> extractAndProcessFirstLine(const std::string& request) {
    std::istringstream request_stream(request);
    std::string line;
    std::getline(request_stream, line);

    std::istringstream line_stream(line);
    std::vector<std::string> result;
    std::string word;
    while (line_stream >> word) {
        result.push_back(word);
    }

    return result;
}

int main(int ac, char **av) {
    (void)ac;
    (void)av;

    chdir("cgi_cpp");

    const char* request_method = getenv("REQUEST_METHOD");
    const char* path_translated = getenv("PATH_TRANSLATED");
    const char* path = av[1];

    if (!request_method || !path_translated) {
        std::cerr << "Error: Required environment variables are not set." << std::endl;
        return 1;
    }

    if (std::string(request_method) == "GET") {
        std::string path = std::string(path_translated);

        std::ifstream log_file("log.txt");
        std::string log_content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
        log_file.close();

        std::cout << constructResponseWithHeader(log_content, "text/plain", "200", "OK");
    } 
    
    if (std::string(request_method) == "POST") {
        appendLineToFile("log.txt", path);
        std::cout << constructResponseWithHeader("ok", "text/plain", "200", "OK");
    }
    
    else {
        std::cout << constructResponseWithHeader("Method not supported: " + std::string(request_method), "text/plain", "405", "Method Not Allowed");
    }

    return 0;
}