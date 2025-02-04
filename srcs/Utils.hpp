#ifndef UTILS_HPP
#define UTILS_HPP

#include "Header.hpp"

bool isDirectory(const string& path);
bool endsWith(const string& str, const string& suffix);
bool startsWith(const string& str, const string& prefix);
bool doesPathExist(const string& resourcePath);
string getAbsolutePath(string filepath);
std::vector<string> splitBy1stInstance(string str, char delimiter);
std::vector<string> splitByNthInstance(string str, char delimiter, int n);
string to_string(int number);
void replaceIfFound(string *haystack, const string& needle, const string& replacement);
string readFileContent(const string& filePath);
string listFiles(const string &path);

#endif