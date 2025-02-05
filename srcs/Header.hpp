#ifndef HEADER_HPP
#define HEADER_HPP

#include <sys/wait.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_PUT "PUT"
#define METHOD_DELETE "DELETE"
#include <sys/stat.h>
#include <cstring>


#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_CSS "text/css"
#define CONTENT_TYPE_JS "application/javascript"
#define CONTENT_TYPE_PNG "image/png"
#define CONTENT_TYPE_JPG "image/jpeg"
#define CONTENT_TYPE_GIF "image/gif"



#define ROOT_RESOURCE_PATH "resources"
#define CGI_BIN_PATH "cgi-bin"
#define PORT 8080
#define BUFFER_SIZE 30000
#define PROTOCOL "HTTP/1.1"
#define CGI_TIMOUT 5

typedef std::string string;
typedef std::map<string, string> stringDict;

#define AUTO_INDEX_ON false

#endif