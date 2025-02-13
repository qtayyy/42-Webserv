#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/Webserv.hpp"



class CGIHandler {
private:
    stringDict envVars;

public:
    CGIHandler();
    ~CGIHandler();
    string waitForCGIResponse(int *pipefd, pid_t pid, int &exitStatus);
    void exec(int *pipefd, const string& cgiScriptPath, const string& queryString, const string& requestedFilepath);
    string handleCgiRequest(const string& cgiScriptPath, const string& queryString, const string requestedFilepath, int &exitStatus);
    string handleCgiRequest(const string &cgiScriptPath, const string &queryString, const string requestedFilepath, string data, int &exitStatus);
    void setEnv(string key, string value);
    string getEnv(string key);
};

#endif