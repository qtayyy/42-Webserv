#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <iomanip>
#include <arpa/inet.h>
#include <stdint.h>
#include <limits>
#include <algorithm>
#include <fstream>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>

#define BLACK "\001\033[30m\002"
#define RED "\001\033[31m\002"
#define GREEN "\001\033[32m\002"
#define YELLOW "\001\033[33m\002"
#define BLUE "\001\033[34m\002"
#define MAGENTA "\001\033[35m\002"
#define CYAN "\001\033[36m\002"
#define WHITE "\001\033[37m\002"
#define GRAY "\001\033[90m\002"
#define LIGHT_GRAY "\001\033[37m\002"
#define BEIGE "\001\033[93m\002"
#define RESET "\001\033[0m\002"

std::string	intToIp(uint32_t ip);

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
#include <sys/stat.h>
#include <cstring>

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_PUT "PUT"
#define METHOD_DELETE "DELETE"


#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_CSS "text/css"
#define CONTENT_TYPE_JS "application/javascript"
#define CONTENT_TYPE_PNG "image/png"
#define CONTENT_TYPE_JPG "image/jpeg"
#define CONTENT_TYPE_GIF "image/gif"
#define CONTENT_TYPE_PDF "application/pdf"


#define ROOT_RESOURCE_PATH "public"
#define CGI_BIN_PATH "cgi-bin"
#define PORT 8080
#define BUFFER_SIZE 30000
#define PROTOCOL "HTTP/1.1"
#define CGI_TIMOUT 5

typedef std::string string;
typedef std::map<string, string> stringDict;
typedef std::vector<string> stringList;

#define AUTO_INDEX_ON true



#endif