#include "CGI.hpp"
#include "HttpException.hpp"

void setEnvironmentVariables(stringDict envVars) {

    std::cout << YELLOW << "Setting env.." << YELLOW << std::endl;
    for (stringDict::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
        setenv(it->first.c_str(), it->second.c_str(), 1);
        std::cout << std::left << YELLOW << std::setw(20) << "\t" + it->first << RESET << it->second << std::endl;
    }
}

CGIHandler::CGIHandler() {

}

CGIHandler::~CGIHandler() {

}


void CGIHandler:: runCGIExecutable(string &cgiScriptPath, string &requestedFilepath) {
    if (endsWith(cgiScriptPath, ".py")) {
        execl("/usr/bin/python3", "python3", cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
    }

    else {
        execl(cgiScriptPath.c_str(), cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
    }
}

string CGIHandler:: handleCgi(string& cgiScriptPath, HttpRequest &request, int &exitStatus, ServerBlock &serverBlock) {
    int inputPipe[2];  // Pipe for sending request body (stdin for CGI)
    int outputPipe[2]; // Pipe for capturing CGI output (stdout from CGI)

    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) 
        throw std::runtime_error("pipe failed: " + string(strerror(errno)));

    string data              = request.getBody();
    string requestedFilepath = request.headerGet("absolute_path");

    // Set environment variables
    this->setEnv("REQUEST_METHOD",  request.getMethod());
    this->setEnv("QUERY_STRING",    request.headerGet("query_string"));
    this->setEnv("SCRIPT_NAME",     cgiScriptPath);
    this->setEnv("SERVER_NAME",     "localhost");
    this->setEnv("SERVER_PORT",     "8080");
    this->setEnv("PATH_INFO",       requestedFilepath);
    this->setEnv("PATH_TRANSLATED", request.headerGet("absolute_path"));
    this->setEnv("CONTENT_LENGTH",  to_string(data.length()));
    this->setEnv("CONTENT_TYPE",    request.headerGet("content_type"));
    setEnvironmentVariables(this->envVars);

    std::cout << YELLOW << "Running CGI..." << RESET << std::endl;

    pid_t pid = fork();

    if (pid < 0) throw std::runtime_error("fork failed: " + string(strerror(errno)));

    if (pid == 0) {
        // **CHILD PROCESS** (Executes CGI)
        close(inputPipe[1]);  // Close unused write end of input pipe
        close(outputPipe[0]); // Close unused read end of output pipe

        // Redirect CGI stdin to inputPipe[0] (reading request body)
        dup2(inputPipe[0], STDIN_FILENO);
        close(inputPipe[0]);  // Close the original read end

        // Redirect CGI stdout to outputPipe[1] (capturing response)
        dup2(outputPipe[1], STDOUT_FILENO);
        close(outputPipe[1]); // Close the original write end

        runCGIExecutable(cgiScriptPath, requestedFilepath);
        
        perror(("execl failed: " + cgiScriptPath).c_str());
        exit(1);
    }
    else {
        // **PARENT PROCESS** (Sends input to CGI)
        close(inputPipe[0]);  // Close unused read end of input pipe
        close(outputPipe[1]); // Close unused write end of output pipe

        // **Write request body to CGI stdin**
        write(inputPipe[1], data.c_str(), data.length());  
        close(inputPipe[1]);  // Close write end to signal EOF

        // **Read CGI response from stdout**
        std::vector<char> responseBuffer;

        char buffer[1024];
        ssize_t bytesRead;
        ssize_t totalBytes = 0;
        while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
            responseBuffer.insert(responseBuffer.end(), buffer, buffer + bytesRead);
            totalBytes += bytesRead;
        }
        std::cout << GREEN << "CGI completed. " << totalBytes << " bytes received from " << basename(cgiScriptPath.c_str()) << RESET << std::endl;
        
        close(outputPipe[0]);  // Close read end after reading

        int status;
        waitpid(pid, &status, 0);

        if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
            std::cout << "Child process terminated by signal: " << signal << std::endl;
        }

        exitStatus = WEXITSTATUS(status);
        return std::string(responseBuffer.begin(), responseBuffer.end());  // Now contains CGI output
    }
    return "";
}



void CGIHandler::setEnv(string key, string value) {
    envVars[key] = value;
}

string CGIHandler::getEnv(string key) {
    return envVars[key];
}