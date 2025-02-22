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


int appendLineToFile(const std::string& path, const std::string& line) {
    std::ofstream file(path.c_str(), std::ios::app);
    if (file.is_open()) {
        file << line << std::endl;
        file.close();
        return 0;
    } 
    
    else {
        std::cerr << "Error: Could not open file at " << path << std::endl;
        return -1;
    }
}

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

std::string css() {
    return 
    "       body {"
    "           font-family: Arial, sans-serif;"
    "           background-color: #f4f4f4;"
    "           margin: 0;"
    "           padding: 20px;"
    "       }"
    "       h1 {"
    "           color: #4CAF50;"
    "       }"
    "       table {"
    "           width: 100%;"
    "           border-collapse: collapse;"
    "           border-radius: 10px;"
    "           overflow: hidden;"
    "           padding: 10px;"
    "       }"
    "       th, td {"
    "           text-align: left;"
    "           background: #fff;"
    "           padding: 10px;"
    "       }"
    "       a {"
    "           color: #1E90FF;"
    "       }"
    "       .folder {"
    "           color:rgb(0, 255, 251);"
    "       }"
    "       .error {"
    "           color: red;"
    "           font-weight: bold;"
    "       }";
}

std::string generateErrorPage(const std::string& title, const std::string& message) {
    std::ostringstream raw_fail_page;
    raw_fail_page << "<html>\n"
                  << "<head>\n"
                  << "<style>\n"
                  << css() << "\n"
                  << "</style>\n"
                  << "</head>\n"
                  << "<body>\n"
                  << "<h1 class=\"error\">" << title << "</h1>\n"
                  << "<p>" << message << "</p>\n"
                  << "</body>\n"
                  << "</html>\n";
    return raw_fail_page.str();
}

std::string generateSuccessPage(const std::string& title, const std::string& message) {
    std::ostringstream raw_success_page;
    raw_success_page << "<html>\n"
                     << "<head>\n"
                     << "<style>\n"
                     << css() << "\n"
                     << "</style>\n"
                     << "</head>\n"
                     << "<body>\n"
                     << "<h1>" << title << "</h1>\n"
                     << "<p>" << message << "</p>\n"
                     << "</body>\n"
                     << "</html>\n";
    return raw_success_page.str();
}

#define RED "\033[31m"
#define RESET "\033[0m"

std::string getHeaderFromResponse(const std::string& response, const std::string& boundary) {
    std::string start_boundary = "--" + boundary;
    size_t start_pos = response.find(start_boundary);
    if (start_pos == std::string::npos) {
        return "";
    }
    start_pos += start_boundary.length();

    size_t header_end_pos = response.find("\r\n\r\n", start_pos);
    if (header_end_pos == std::string::npos) {
        return "";
    }

    return response.substr(start_pos, header_end_pos - start_pos);
}


std::string getBoundary(const std::string& contentType) {
    std::string boundaryPrefix = "boundary=";
    size_t boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos != std::string::npos) {
        return contentType.substr(boundaryPos + boundaryPrefix.length());
    }
    return "";
}

#include <map>

std::map<std::string, std::string> extractHeadersFromHeaderString(const std::string& headerString) {
    std::istringstream headerStream(headerString);
    std::map<std::string, std::string> headers;
    std::string line;
    while (std::getline(headerStream, line)) {

        std::cerr << "line: " << line << std::endl;
        size_t delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            headers[key] = value;
        }
    }
    return headers;
}

std::string to_string(int i) {
    std::ostringstream oss;
    oss << i;
    return oss.str();
}

int main(int ac, char **av) {
    (void)ac;
    (void)av;

    chdir("cgi_cpp");

    const char* request_method = getenv("REQUEST_METHOD");
    const char* path_translated = getenv("PATH_TRANSLATED");
    const char* path = av[1];
    const char* content_type = getenv("CONTENT_TYPE");


    (void)path;

    if (!request_method || !path_translated || !content_type) {
        std::cerr << "Error: Required environment variables are not set." << std::endl;
        return 1;
    }

    std::string fullPath = "";

    if (std::string(request_method) == "GET") {
        std::string path = std::string(path_translated);

        std::ifstream log_file("log.txt");
        std::string log_content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
        log_file.close();

        std::cout << constructResponseWithHeader(log_content, "text/plain", "200", "OK");
    } 
    
    if (std::string(request_method) == "POST") {
        std::string postData;
        std::string line;
        while (std::getline(std::cin, line)) {
            postData += line + "\n";
        }

        std::string boundary = getBoundary(content_type ? content_type : "");
        if (boundary.empty()) {
            std::cerr << "Error: Boundary not found in Content-Type header." << std::endl;
            return 1;
        }

        int status = appendLineToFile("log.txt", path_translated);
        std::map<std::string, std::string> headerItems = extractHeadersFromHeaderString(getHeaderFromResponse(postData, boundary));

        std::string output = "";
        for (std::map<std::string, std::string>::iterator it = headerItems.begin(); it != headerItems.end(); ++it) {
            output += it->first + ": " + it->second + "\n";
        }

        output += to_string(headerItems.size());

        std::cout << constructResponseWithHeader(getHeaderFromResponse(postData, boundary), "text/plain", "200", "OK");
        return 0;
        
        if (status == 0)
            std::cout << constructResponseWithHeader(generateSuccessPage("File logged successfully", path_translated), "text/html", "200", "OK");
        else if (status == -1)
            std::cout << constructResponseWithHeader(generateErrorPage("File could not be logged", path_translated), "text/html", "500", "Internal Server Error");


    }
    
    else {
        std::cout << constructResponseWithHeader("Method not supported: " + std::string(request_method), "text/plain", "405", "Method Not Allowed");
    }

    return 0;
}


// bool doesPathExist(const std::string& path) {
//     struct stat buffer;
//     return (stat(path.c_str(), &buffer) == 0);
// }


// bool isDirectory(const std::string& path) {
//     struct stat buffer;
//     if (stat(path.c_str(), &buffer) != 0) {
//         return false;
//     }
//     return S_ISDIR(buffer.st_mode);
// }


// void createFile(const std::string& path, const std::string& content) {
//     std::ofstream file(path.c_str());
//     if (file.is_open()) {
//         file << content;
//         file.close();
//         std::cout << "File created successfully at " << path << std::endl;
//     } else {
//         std::cerr << "Error: Could not create file at " << path << std::endl;
//     }
// }


// std::string constructHttpResponseFromFile(const std::string& path) {
//     std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

//     if (!file.is_open()) {
//         std::cerr << "Error: Could not open file " << path << std::endl;
//         return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
//     }

//     // Read the entire file content into a string
//     std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     file.close();

//     std::string content_type = getContentType(path);

//     return constructResponseWithHeader(file_content, content_type, "200", "OK");
// }