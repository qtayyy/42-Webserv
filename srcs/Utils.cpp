#include "Utils.hpp"
#include "HttpException.hpp"
#include "Log.hpp"

void createFileWithContents(const string& filePath, const string& contents) {
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
    if (str.size() == 0 || prefix.size() == 0) 
        return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool isPathExist(const string& resourcePath) {
    struct stat buffer;
    return (stat(resourcePath.c_str(), &buffer) == 0);
}

string makeAbsPath(string filepath) {
    string fullPath = getcwd(NULL, 0);
    fullPath = joinPaths(fullPath, filepath);
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

stringList splitString(string str, char delimiter) {
    std::vector<string> tokens;
    string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int getFileSize(const string& filePath) {
    struct stat stat_buf;
    int rc = stat(filePath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

string getDirectory(const string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != string::npos) {
        return fullPath.substr(0, pos);
    }
    return "";
}

string appendPaths(const string& path1, const string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;

    string result = path1;
    if (result[result.size() - 1] == '/' || result[result.size() - 1] == '\\') {
        result.erase(result.size() - 1);
    }

    if (path2[0] == '/' || path2[0] == '\\')
        result += path2;
    else
        result += '/' + path2;

    return result;
}

string readFileContent(const string& filePath) {
    std::ifstream file(filePath.c_str()); // Open the file
    if (!file.is_open()) {
        if (!isPathExist(filePath)) {
            throw HttpException(404, "error reading from " + filePath); // Assuming file not found
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

std::vector<string> listFiles(const string &path) {
    struct dirent *entry;
    DIR *dir = opendir(path.c_str());
    std::vector<string> files;
    if (dir == NULL) {
        std::cerr << "Error opening directory" << std::endl;
        return files;
    }
    while ((entry = readdir(dir)) != NULL) {
        files.push_back(entry->d_name);
    }
    closedir(dir);
    return files;
}

string getBasename(const string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != string::npos) {
        return fullPath.substr(pos + 1);
    }
    return fullPath;
}

string getDirname(const string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != string::npos) {
        return fullPath.substr(0, pos);
    }
    return ".";
}


int getTerminalWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

bool isAnsiEscapeStart(char c) {
    return c == '\033' || c == '\x1B' || c == '\001' || c == '\002';
}

std::size_t visibleLength(const string& str) {
	std::size_t length = 0;
	bool inEscape = false;

	for (std::size_t i = 0; i < str.length(); ++i) {
		if (!inEscape) {
			if (isAnsiEscapeStart(str[i]) && i + 1 < str.length() && str[i + 1] == '[') {
				inEscape = true;
				++i; // skip the '['
			} else {
				++length;
			}
		} else {
			// We're in an escape sequence. End when we hit a letter.
			if (std::isalpha(static_cast<unsigned char>(str[i]))) {
				inEscape = false;
			}
		}
	}

	return length;
}

void printBorderedBox(const string& message, const string& title) {
	int width = getTerminalWidth();
	std::istringstream stream(message);
	string line;

	string top = " " + string(width - 2, '_') + " ";
	string bottom = " " + string(width - 2, '_') + " ";
	std::cout << top << std::endl;

	// Print the title in the center
	if (!title.empty()) {
		int titlePadding = (width - 2 - visibleLength(title)) / 2;
		std::cout << "│" << string(titlePadding, ' ') << title 
				  << string(width - 2 - titlePadding - visibleLength(title), ' ') << "│" << std::endl;
	}

	while (std::getline(stream, line)) {
		while (visibleLength(line) > static_cast<size_t>(width - 4)) {
			size_t cutOff = width - 4;
			size_t actualCutOff = 0;
			size_t visibleCount = 0;

			// Find the actual cutoff point considering escape sequences
			for (size_t i = 0; i < line.size(); ++i) {
				if (isAnsiEscapeStart(line[i])) {
					while (i < line.size() && line[i] != 'm') ++i;
				} else {
					visibleCount++;
					if (visibleCount == cutOff) {
						actualCutOff = i + 1;
						break;
					}
				}
			}

			std::cout << "│ " << line.substr(0, actualCutOff) << string(width - 4 - visibleLength(line.substr(0, actualCutOff)), ' ') << " │" << std::endl;
			line = line.substr(actualCutOff);
		}
		int padding = width - 4 - visibleLength(line); // 4 = 2 for '|' + 2 for space on both sides
		std::cout << "│ " << line << string(padding, ' ') << " │" << std::endl;
	}

	std::cout << bottom << std::endl << std::endl;
}

string urlEncode(const string& value) {
    std::ostringstream encoded;
    for (string::const_iterator it = value.begin(); it != value.end(); ++it) {
        unsigned char c = static_cast<unsigned char>(*it);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else if (c == ' ') {
            encoded << "%20";  // or '+' if you want form-style encoding
        } else {
            encoded << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)c;
        }
    }
    return encoded.str();
}

// Function to URL-decode a string
string urlDecode(const string& value) {
    std::ostringstream decoded;
    for (size_t i = 0; i < value.length(); ++i) {
        if (value[i] == '%' && i + 2 < value.length()) {
            int hex = 0;
            std::istringstream(value.substr(i + 1, 2)) >> std::hex >> hex;
            decoded << static_cast<char>(hex);
            i += 2;
        } else if (value[i] == '+') {
            decoded << ' ';  // optional: handle '+' as space for form-encoded input
        } else {
            decoded << value[i];
        }
    }
    return decoded.str();
}

string joinPaths(const string& base, const string& sub) {
    if (base.empty()) 
        return "/" + sub;
    if (sub == "/") 
        return base;
    if (base[base.size() - 1] == '/')
        return base + (sub[0] == '/' ? sub.substr(1) : sub);
    else
        return base + (sub[0] == '/' ? sub : "/" + sub);
}

string generateRandomID(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static const size_t charsetSize = sizeof(charset) - 1;

    string randomID;
    srand(static_cast<unsigned int>(clock()));

    for (size_t i = 0; i < length; ++i)
        randomID += charset[rand() % charsetSize];

    return randomID;
}

string currentDateTime() {
    // Get the current date and time
    time_t now = time(NULL);
    tm* localTime = localtime(&now);
    char timeBuffer[100];
    strftime(timeBuffer, sizeof(timeBuffer), "%I:%M%p %d-%m-%Y", localTime); // %I for 12-hour format, %p for AM/PM
    return timeBuffer;
}

string formatTime(const string& format) {
    // Get the current date and time
    time_t now = time(NULL);
    tm* localTime = localtime(&now);
    char timeBuffer[256];
    strftime(timeBuffer, sizeof(timeBuffer), format.c_str(), localTime); // Use the provided format string
    return string(timeBuffer);
}

string sanitizeFilename(string str) {
    string sanitized;
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.' || c == ' ' || c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}') {
            sanitized += c;
        }
    }
    return sanitized;
}

string generateLogFileName(const string &folder, const string& prefix) {
    string outputFolder = folder + (sanitizeFilename(string(prefix)) + " [" + currentDateTime() + "] " + generateRandomID(5) + ".log");
    
    LogStream::log("log_trace.log", std::ios::app) << " [" + currentDateTime() + "] " << "Log file created: " << outputFolder << std::endl;
    return outputFolder;
}