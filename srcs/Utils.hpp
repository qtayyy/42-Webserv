#ifndef UTILS_HPP
#define UTILS_HPP

#include "../includes/Webserv.hpp"
#include <string>
#include <cctype>
#include <sys/ioctl.h>
#include <unistd.h>

bool isDirectory(const string& path);
bool endsWith(const string& str, const string& suffix);
bool startsWith(const string& str, const string& prefix);
bool isPathExist(const string& resourcePath);
string getAbsolutePath(string filepath);
std::vector<string> splitBy1stInstance(string str, char delimiter);
std::vector<string> splitByNthInstance(string str, char delimiter, int n);
string to_string(int number);
void replaceIfFound(string *haystack, const string& needle, const string& replacement);
string readFileContent(const string& filePath);
std::vector<string> listFiles(const string &path);
void createFileWithContents(const std::string& filePath, const std::string& contents);
stringList splitString(string str, char delimiter);
int getFileSize(const std::string& filePath);
std::string getDirectory(const std::string& fullPath);    
string appendPaths(const std::string& path1, const std::string& path2);
    
int getTerminalWidth();
bool isAnsiEscapeStart(char c);
std::size_t visibleLength(const std::string& str);
void printBorderedBox(const std::string& message, const std::string& title);
    
std::string urlEncode(const std::string& value);
std::string urlDecode(const std::string& value);
    
std::string joinPaths(const std::string& base, const std::string& sub);
#endif