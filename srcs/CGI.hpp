#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/Webserv.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerBlock.hpp"
#include "Log.hpp"

#include "Utils.hpp"

class HttpResponse;

class CGIHandler {
private:
    stringDict envVars;

public:
    CGIHandler();
    ~CGIHandler();
    string waitForCGIResponse(int *pipefd, pid_t pid, int &exitStatus);
    void exec(int *pipefd, const string& cgiScriptPath, const string& queryString, const string& requestedFilepath);
    void runCGIExecutable(string &path, string &requestedFilepath);
    
    string handleCgi(
        string        &cgiScriptPath, 
        HttpRequest   &request, 
        int           &exitStatus, 
        LocationBlock &serverBlock, 
        HttpResponse  &response
    );
    
    void setEnv(string key, string value);
    string getEnv(string key);
};

#endif