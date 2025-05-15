#include "cgi_exec.hpp"

string css() {
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

string to_string(int i) {
    std::ostringstream oss;
    oss << i;
    return oss.str();
}

string getContentType(const string& resourcePath) {
    string resource_type = resourcePath.substr(resourcePath.find_last_of(".") + 1);
    if      (resource_type == "html") return "text/html";
    else if (resource_type == "css")  return "text/css";
    else if (resource_type == "js")   return "text/javascript";
    else if (resource_type == "jpeg"  || resource_type == "jpg") return "image/jpg";
    else if (resource_type == "png")  return "image/png";
    else if (resource_type == "pdf")  return "application/pdf";
    else                              return "text/plain";
}



string urlDecode(const string &encoded) {
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
        decoded << (encoded[i] == '+') ? ' ' : encoded[i];
    }
    return decoded.str();
}

string constructResponse(
        const string &body, 
        const string &content_type, 
        const string &status_code, 
        const string &status_message) {
    std::ostringstream headers;
    headers << "HTTP/1.1 " << status_code << " " << status_message << "\n"
            << "Content-Type: " + content_type + "\n"
            << "Connection: close\n"
            << "Content-Length: " << body.size() << "\n\n";
    return headers.str() + body;
}

int appendLineToFile(const string& path, const string& line) {
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

string generateStatusPage(const string& title, const string& message, bool isError) {
    std::stringstream raw_fail_page;
    raw_fail_page << "<html>\n"
                  << "<head>\n"
                  << "<style>\n"
                  << css() << "\n"
                  << "</style>\n"
                  << "</head>\n"
                  << "<body>\n"
                  << (isError ? ("<h1 class=\"error\">") : "<h1>") << title << "</h1>\n"
                  << "<p>" << message << "</p>\n"
                  << "</body>\n"
                  << "</html>\n";
    return raw_fail_page.str();
}

string extractHeader(const string& response, const string& boundary) {
    string start_boundary = "--" + boundary;
    size_t start_pos = response.find(start_boundary);
    if (start_pos == string::npos)
        return "";
    start_pos += start_boundary.length();

    size_t header_end_pos = response.find("\r\n\r\n", start_pos);
    if (header_end_pos == string::npos)
        return "";

    return response.substr(start_pos, header_end_pos - start_pos);
}

string extractBoundary(const string& contentType) {
    string boundaryPrefix = "boundary=";
    size_t boundaryPos    = contentType.find(boundaryPrefix);

    if (boundaryPos != string::npos)
        return contentType.substr(boundaryPos + boundaryPrefix.length());
    return "";
}

std::map<string, string> extractHeadersFromHeaderString(const string& headerString) {
    std::istringstream headerStream(headerString);
    std::map<string, string> headers;
    string line;
    while (std::getline(headerStream, line)) {

        size_t delimiterPos = line.find(": ");
        if (delimiterPos != string::npos) {
            string key = line.substr(0, delimiterPos);
            string value = line.substr(delimiterPos + 2);
            headers[key] = value;
        }
    }
    return headers;
}

string readFileContent(const string& filePath) {
    std::ifstream file(filePath.c_str()); // Open the file
    if (!file.is_open()) 
        return "";

    string content;
    string line;
    while (std::getline(file, line)) 
        content += line + '\n'; // Append each line to the content with a newline character

    file.close(); // Close the file
    return content;
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

    string fullPath = "";

    if (string(request_method) == "GET") {
        string path = string(path_translated);

        std::cout << constructResponse(readFileContent("log.txt"), "text/plain", "200", "OK");
    }
    
    else if (string(request_method) == "POST") {
        string postData;
        string line;
        while (std::getline(std::cin, line)) 
            postData += line + "\n";

        string boundary = extractBoundary(content_type ? content_type : "");
        if (boundary.empty()) {
            std::cerr << "Error: Boundary not found in Content-Type header." << std::endl;
            return 1;
        }

        int status = appendLineToFile("log.txt", path_translated);
        std::map<string, string> headerItems = extractHeadersFromHeaderString(extractHeader(postData, boundary));

        string filename;
        size_t chunk_start = postData.find("--" + boundary);
        if (chunk_start != string::npos) {
            size_t filename_pos = postData.find("filename=\"", chunk_start);
            if (filename_pos != string::npos) {
                filename_pos += 10; // Move past 'filename="'
                size_t filename_end = postData.find("\"", filename_pos);
                if (filename_end != string::npos)
                    filename = postData.substr(filename_pos, filename_end - filename_pos);
            } 
        }
        else {
            std::cerr << "Error: Chunk start not found in the POST data." << std::endl;
            return 1;
        }


        appendLineToFile("log.txt", "filename uploaded: " + filename);

        std::cout << constructResponse(generateStatusPage("File logged successfully", path_translated, false), "text/html", "200", "OK");
        return 0;
        
        if (status == 0)
            std::cout << constructResponse(generateStatusPage("File logged successfully", path_translated, false), "text/html", "200", "OK");
        else if (status == -1)
            std::cout << constructResponse(generateStatusPage("File could not be logged", path_translated, true), "text/html", "500", "Internal Server Error");
    }
    
    else {
        std::cout << constructResponse("Method not supported: " + string(request_method), "text/plain", "405", "Method Not Allowed");
    }

    return 0;
}
   // string output = "";
        // for (std::map<string, string>::iterator it = headerItems.begin(); it != headerItems.end(); ++it)
        //     output += it->first + ": " + it->second + "\n";

        // output += to_string(headerItems.size());

// bool doesPathExist(const string& path) {
//     struct stat buffer;
//     return (stat(path.c_str(), &buffer) == 0);
// }


// bool isDirectory(const string& path) {
//     struct stat buffer;
//     if (stat(path.c_str(), &buffer) != 0) {
//         return false;
//     }
//     return S_ISDIR(buffer.st_mode);
// }


// void createFile(const string& path, const string& content) {
//     std::ofstream file(path.c_str());
//     if (file.is_open()) {
//         file << content;
//         file.close();
//         std::cout << "File created successfully at " << path << std::endl;
//     } else {
//         std::cerr << "Error: Could not create file at " << path << std::endl;
//     }
// }


// string constructHttpResponseFromFile(const string& path) {
//     std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

//     if (!file.is_open()) {
//         std::cerr << "Error: Could not open file " << path << std::endl;
//         return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
//     }

//     // Read the entire file content into a string
//     string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     file.close();

//     string content_type = getContentType(path);

//     return constructResponseWithHeader(file_content, content_type, "200", "OK");
// }