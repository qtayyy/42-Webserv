#include "CGI.hpp"
#include "HttpException.hpp"

string CGIHandler::waitForCGIResponse(int *pipefd, pid_t pid, int &exitStatus) {
    // Parent process
    string response = "";

    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);

    int status;
    waitpid(pid, &status, 0);

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        std::cout<< RED << "Child process was terminated by signal: " << signal << RESET << std::endl;
        throw HttpException(500);
    }

    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }
    close(pipefd[0]);

    exitStatus = WEXITSTATUS(status);
    return response;
}

void setEnvironmentVariables(stringDict envVars) {
    for (stringDict::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
        setenv(it->first.c_str(), it->second.c_str(), 1);
    }
}

CGIHandler::CGIHandler() {

}

CGIHandler::~CGIHandler() {

}

void CGIHandler::exec (
    int *pipefd, 
    const string& cgiScriptPath, 
    const string& queryString, 
    const string& requestedFilepath) {
    // Child process
    close(pipefd[0]); 
    setEnvironmentVariables(this->envVars);

    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    execl("/usr/bin/python3", "python3", cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
    
    perror(("execl failed: " + cgiScriptPath).c_str());
    exit(1);
}



string CGIHandler::handleCgiRequest(const string& cgiScriptPath, const string& queryString, const string requestedFilepath, int &exitStatus) {
    int pipefd[2];

    if (pipe(pipefd) == -1) 
        throw std::runtime_error("pipe failed: " + string(strerror(errno)));

    pid_t pid = fork();

    if (pid < 0) throw std::runtime_error("fork failed: " + string(strerror(errno)));

    if (pid == 0) {
        this->exec(pipefd, cgiScriptPath, queryString, requestedFilepath);
    }
    else {
        // pid_t monitorPid = fork();

        // if (monitorPid < 0) 
        //     throw std::runtime_error("fork failed: " + string(strerror(errno)));

        // if (monitorPid == 0) {
        //     // Detach the monitor process
        //     if (setsid() < 0) {
        //         throw std::runtime_error("setsid failed: " + string(strerror(errno)));
        //     }

        //     // Monitor process
        //     sleep(CGI_TIMOUT);
        //     std::ofstream logFile("/home/cooper/coreProgram/webserv_correct/webserv_redo/resources/log.txt");
        //     if (kill(pid, 0) == 0) {
        //         logFile << "Child process is still running.\n";
        //         kill(pid, SIGKILL);
        //         logFile.close();
        //         exit(504); // Exit with 504 status code
        //     } else {
        //         logFile << "Child process has already terminated.\n";
        //     }
        //     logFile.close();
        //     exit(0);
        // }
        // Main process continues without waiting for the monitor process
        return waitForCGIResponse(pipefd, pid, exitStatus);
    }

    return "";
}

void CGIHandler::setEnv(string key, string value) {
    envVars[key] = value;
}

string CGIHandler::getEnv(string key) {
    return envVars[key];
}