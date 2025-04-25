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
    if (str.size() == 0 || prefix.size() == 0) 
        return false;
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

stringList splitString(string str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int getFileSize(const std::string& filePath) {
    struct stat stat_buf;
    int rc = stat(filePath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

std::string getDirectory(const std::string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return fullPath.substr(0, pos);
    }
    return "";
}

string appendPaths(const std::string& path1, const std::string& path2) {
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
        if (!doesPathExist(filePath)) {
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



int getTerminalWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

bool isAnsiEscapeStart(char c) {
    return c == '\033' || c == '\x1B' || c == '\001' || c == '\002';
}

std::size_t visibleLength(const std::string& str) {
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

void printBorderedBox(const std::string& message, const std::string& title) {
	int width = getTerminalWidth();
	std::istringstream stream(message);
	std::string line;

	std::string top = " " + std::string(width - 2, '_') + " ";
	std::string bottom = " " + std::string(width - 2, '_') + " ";
	std::cout << top << std::endl;

	// Print the title in the center
	if (!title.empty()) {
		int titlePadding = (width - 2 - visibleLength(title)) / 2;
		std::cout << "│" << std::string(titlePadding, ' ') << title 
				  << std::string(width - 2 - titlePadding - visibleLength(title), ' ') << "│" << std::endl;
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

			std::cout << "│ " << line.substr(0, actualCutOff) << std::string(width - 4 - visibleLength(line.substr(0, actualCutOff)), ' ') << " │" << std::endl;
			line = line.substr(actualCutOff);
		}
		int padding = width - 4 - visibleLength(line); // 4 = 2 for '|' + 2 for space on both sides
		std::cout << "│ " << line << std::string(padding, ' ') << " │" << std::endl;
	}

	std::cout << bottom << std::endl << std::endl;
}

std::string urlEncode(const std::string& value) {
    std::ostringstream encoded;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
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
std::string urlDecode(const std::string& value) {
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