/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shechong <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 14:39:13 by shechong          #+#    #+#             */
/*   Updated: 2025/05/16 14:39:14 by shechong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../includes/Webserv.hpp"
#include <string>
#include <cctype>
#include <sys/ioctl.h>
#include <unistd.h>
#include "HttpException.hpp"
#include "Log.hpp"

bool isDirectory(const string& path);
bool endsWith(const string& str, const string& suffix);
bool startsWith(const string& str, const string& prefix);
bool isPathExist(const string& resourcePath);
string makeAbsPath(string filepath);
std::vector<string> splitBy1stInstance(string str, char delimiter);
std::vector<string> splitByNthInstance(string str, char delimiter, int n);
string to_string(int number);
void replaceIfFound(string *haystack, const string& needle, const string& replacement);
string readFileContent(const string& filePath);
std::vector<string> listFiles(const string &path);
void createFileWithContents(const string& filePath, const string& contents);
stringList splitString(string str, char delimiter);
int getFileSize(const string& filePath);
string getDirectory(const string& fullPath);    
string appendPaths(const string& path1, const string& path2);
string generateRandomID(size_t length);
int getTerminalWidth();
bool isAnsiEscapeStart(char c);
std::size_t visibleLength(const string& str);
void printBorderedBox(const string& message, const string& title);
    
string formatTime(const string& format);
string urlEncode(const string& value);
string urlDecode(const string& value);
    
string getBasename(const string& fullPath);
string getDirname(const string& fullPath);

string joinPaths(const string& base, const string& sub);
string currentDateTime();

string sanitizeFilename(string str);

string generateLogFileName(const string &folder, const string &category, const string& prefix);


#endif
