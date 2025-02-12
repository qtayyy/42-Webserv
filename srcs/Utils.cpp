#include "Utils.hpp"
#include "HttpException.hpp"


void createFileWithContents(const std::string& filePath, const std::string& contents) {
    std::ofstream outFile(filePath.c_str());
    if (outFile.is_open()) {
        outFile << contents;
        outFile.close();
        std::cout << "File created and contents written successfully." << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
    }
}

bool isDirectory(const string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0; // Check if it is a directory
}

bool endsWith(const string& str, const string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

bool startsWith(const string& str, const string& prefix) {
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool doesPathExist(const string& resourcePath) {
    struct stat buffer;
    return (stat(resourcePath.c_str(), &buffer) == 0);
}

string getAbsolutePath(string filepath) {
    string fullPath = getcwd(NULL, 0);
    fullPath += "/" + filepath;
    return fullPath;
}

std::vector<string> splitBy1stInstance(string str, char delimiter) {
    size_t pos = str.find(delimiter);
    std::vector<string> result;
    result.push_back(str.substr(0, pos));
    result.push_back(str.substr(pos + 1));
    return result;
}

std::vector<string> splitByNthInstance(string str, char delimiter, int n) {
    size_t pos = 0;
    int count = 0;
    while (count < n && (pos = str.find(delimiter, pos)) != string::npos) {
        ++count;
        if (count < n)
            ++pos;
    }
    std::vector<string> result;
    if (pos != string::npos) {
        result.push_back(str.substr(0, pos));
        result.push_back(str.substr(pos + 1));
    } else {
        result.push_back(str);
        result.push_back("");
    }
    return result;
}

string to_string(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

void replaceIfFound(string *haystack, const string& needle, const string& replacement) {
    size_t pos = haystack->find(needle);
    if (pos != string::npos)
        haystack->replace(pos, needle.size(), replacement);
}

string readFileContent(const string& filePath) {
    std::ifstream file(filePath.c_str()); // Open the file
    if (!file.is_open()) {
        if (!doesPathExist(filePath)) {
            throw HttpException(404); // Assuming file not found
        } else if (file.fail()) {
            throw HttpException(403); // Assuming permission denied
        } else {
            throw HttpException(500); // General error
        }
    }

    string content;
    string line;
    while (std::getline(file, line)) {
        content += line + '\n'; // Append each line to the content with a newline character
    }

    file.close(); // Close the file
    return content;
}

string listFiles(const string &path) {
    struct dirent *entry;
    DIR *dir = opendir(path.c_str());
    string files = "";
    if (dir == NULL) {
        std::cerr << "Error opening directory" << std::endl;
        exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        files += entry->d_name;
        files += "\n";
    }
    return files;
}