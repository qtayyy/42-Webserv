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

#endif