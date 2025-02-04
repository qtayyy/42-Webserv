#include "HttpRequest.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "StatusHandler.hpp"
#include "Utils.hpp"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "HttpResponse.hpp"

// string getUrlFromRequest(const string& request) {
//     std::istringstream requestStream(request);
//     string method, url, protocol;
//     requestStream >> method >> url >> protocol;

//     return url;
// }

HttpRequest mockRequest(string path, string path_info) {
    HttpRequest request;

    request.setParam("path", path);
    request.setParam("path_info", path_info);
    request.setParam("query_string", "name=John&age=30");
    request.setParam("method", "GET");

    return request;
}

// int main(int ac, char **av) {

//     // Change to root directory
//     chdir("..");
//     string root = getcwd(NULL, 0);

//     int server_fd, new_socket;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);
    

//     // Initialize request
//     string path_info = "test.csv";
//     if (av[2] != NULL) 
//         path_info = av[2];
//     HttpRequest request = mockRequest(av[1], path_info);
//     request.printInfo();


//     // Creating socket file descriptor
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { perror("socket failed"); exit(EXIT_FAILURE); }

//     // Forcefully attaching socket to the port 8080
//     int opt = 1;
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { perror("setsockopt"); exit(EXIT_FAILURE); }

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(PORT);

//     // Forcefully attaching socket to the port 8080
//     if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { perror("bind failed"); exit(EXIT_FAILURE); }
    
//     // Listen
//     if (listen(server_fd, 3) < 0) { perror("listen"); exit(EXIT_FAILURE); }

//     // wait for connection
//     while (true) {
//         std::cout << "Waiting for a connection..." << std::endl;

//         // Connect
//         if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) { 
//             perror("accept"); exit(EXIT_FAILURE); 
//         }
            
//         char buffer[BUFFER_SIZE] = {0};
//         int valread = read(new_socket, buffer, BUFFER_SIZE);

//         //std::cout << BLUE << "Received request:\n" << buffer << RESET << std::endl ;

//         HttpResponse response = HttpResponse::createHttpResponse(request);
//         std::cout << GREEN << "Response:\n" << response.getFinalResponseMsg() << RESET << std::endl;
//         send(new_socket, response.getFinalResponseMsg().c_str(), response.getFinalResponseMsg().size(), 0);
//         close(new_socket);
//     }
// }
