#include "CGI.hpp"
#include "HttpException.hpp"

void setEnvironmentVariables(stringDict envVars) {
    for (stringDict::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
        setenv(it->first.c_str(), it->second.c_str(), 1);
    }
}

CGIHandler::CGIHandler() {

}

CGIHandler::~CGIHandler() {

}

// string CGIHandler::waitForCGIResponse(int *pipefd, pid_t pid, int &exitStatus) {
//     // Parent process
//     string response = "";

//     close(pipefd[1]);
//     dup2(pipefd[0], STDIN_FILENO);

//     int status;
//     waitpid(pid, &status, 0);

//     if (WIFSIGNALED(status)) {
//         int signal = WTERMSIG(status);
//         std::cout<< RED << "Child process was terminated by signal: " << signal << RESET << std::endl;
//         throw HttpException(500);
//     }

//     char buffer[1024];
//     ssize_t bytesRead;
//     while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
//         buffer[bytesRead] = '\0';
//         response += buffer;
//     }
//     close(pipefd[0]);

//     exitStatus = WEXITSTATUS(status);
//     return response;
// }

// void CGIHandler::exec(
//     int *pipefd, 
//     const string& cgiScriptPath, 
//     const string& queryString, 
//     const string& requestedFilepath) {
//     // Child process
//     close(pipefd[0]); 

//     dup2(pipefd[1], STDIN_FILENO);  // Redirect stdin to the write end of the pipe
//     write(pipefd[1], "hallo", 5); // Write data to the pipe

//     execl("/usr/bin/python3", "python3", cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
    
//     perror(("execl failed: " + cgiScriptPath).c_str());
//     exit(1);
// }



string CGIHandler:: handleCgiRequest(const string& cgiScriptPath, HttpRequest &request, int &exitStatus, ServerBlock &serverBlock) {
    int inputPipe[2];  // Pipe for sending request body (stdin for CGI)
    int outputPipe[2]; // Pipe for capturing CGI output (stdout from CGI)

    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) 
        throw std::runtime_error("pipe failed: " + string(strerror(errno)));

    string data = readFileContent("test_post_request");

    // Set environment variables
    this->setEnv("REQUEST_METHOD",  request.headerGet("method"));
    this->setEnv("QUERY_STRING",    request.headerGet("query_string"));
    this->setEnv("SCRIPT_NAME",     cgiScriptPath);
    this->setEnv("SERVER_NAME",     "localhost");
    this->setEnv("SERVER_PORT",     "8080");
    this->setEnv("PATH_INFO",       request.headerGet("path_info"));
    this->setEnv("PATH_TRANSLATED", request.headerGet("path"));
    this->setEnv("CONTENT_LENGTH",  to_string(data.length()));
    this->setEnv("CONTENT_TYPE",    request.headerGet("content_type"));
    setEnvironmentVariables(this->envVars);

    pid_t pid = fork();
    string requestedFilepath = request.headerGet("path_info");

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

        if (endsWith(cgiScriptPath, ".py")) {
            execl("/usr/bin/python3", "python3", cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
        }

        else {
            execl(cgiScriptPath.c_str(), cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
        }
        
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

        std::cout << "TOTAL BYTES" << totalBytes << std::endl;
        std::cout << "LENGTH RESPONSE" << responseBuffer.size() << std::endl;
        
        close(outputPipe[0]);  // Close read end after reading

        int status;
        waitpid(pid, &status, 0);

        if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
            std::cout << "Child process terminated by signal: " << signal << std::endl;
            throw HttpException(500);
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