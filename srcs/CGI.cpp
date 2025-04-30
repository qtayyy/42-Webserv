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

string CGIHandler::handleCgi(string& cgiScriptPath, HttpRequest &request, int &exitStatus, LocationBlock &serverBlock) {
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
    this->setEnv("CONTENT_TYPE",    request.headerGet("Content-Type"));
    setEnvironmentVariables(this->envVars);

    std::cout << YELLOW << "Running CGI..." << RESET << std::endl;
    std::cout << YELLOW << "Bytes to be written to CGI stdin: " << data.length() << RESET << std::endl;

    // std::cout << "WRITING: " << data << std::endl;
    std::cout << "Input PIPE: " << inputPipe[0] << ", " << inputPipe[1] << " | Output PIPE: " << outputPipe[0] << ", " << outputPipe[1] << std::endl;

    // Write the request body to an output file for debugging or logging purposes
    std::ofstream requestFile("cgi_request.txt", std::ios::out | std::ios::trunc);
    if (!requestFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing CGI request");
    }
    std::cout << "Request bytes: " << data.length() << std::endl;

    requestFile << data;
    requestFile.close();

    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to a closed pipe

    pid_t pid = fork();

    if (pid < 0) throw std::runtime_error("fork failed: " + string(strerror(errno)));

    if (pid == 0) {
        // **CHILD PROCESS** (Executes CGI)
        close(inputPipe[1]);  // Close unused write end of input pipe
        close(outputPipe[0]); // Close unused read end of output pipe

        // Redirect CGI stdin to inputPipe[0] (reading request body)
        if (dup2(inputPipe[0], STDIN_FILENO) == -1) {
            perror("Failed to redirect stdin for CGI");
            exit(1);
        }
        if (dup2(outputPipe[1], STDOUT_FILENO) == -1) {
            perror("Failed to redirect stdout for CGI");
            exit(1);
        }
        close(inputPipe[0]);
        close(outputPipe[1]);
        std::string rootPath = serverBlock.getRoot();
        runCGIExecutable(cgiScriptPath, rootPath);
        
        perror(("execl failed: " + cgiScriptPath).c_str());
        exit(1);
    }
    else {
        // **PARENT PROCESS** (Sends input to CGI)
        close(inputPipe[0]);  // Close unused read end of input pipe
        close(outputPipe[1]); // Close unused write end of output pipe

        // **Write request body to CGI stdin**
        ssize_t bytesWritten = 0;
        ssize_t length = data.length();
        while (bytesWritten < length) {
            ssize_t result = write(inputPipe[1], data.c_str() + bytesWritten, length - bytesWritten);
            if (result == -1) {
                if (errno == EPIPE) {
                    std::cerr << RED << "Broken pipe: CGI process terminated prematurely." << RESET << std::endl;
                    break;
                }
                close(inputPipe[1]);
                throw std::runtime_error("Failed to write to CGI stdin: " + string(strerror(errno)));
            }
            bytesWritten += result;
            std::cout << "Wrote " << result << " bytes to CGI stdin" << std::endl;
        }
        close(inputPipe[1]);  // Close write end to signal EOF

        // **Read CGI response from stdout**
        std::vector<char> responseBuffer;
        char              buffer[1024];
        ssize_t           bytesRead;
        ssize_t           totalBytes = 0;

        // Open a file to write the CGI output
        std::ofstream outputFile("cgi_output.txt", std::ios::out | std::ios::trunc);
        if (!outputFile.is_open()) {
            close(outputPipe[0]);
            throw std::runtime_error("Failed to open file for writing CGI output");
        }

        while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
            responseBuffer.insert(responseBuffer.end(), buffer, buffer + bytesRead);
            totalBytes += bytesRead;

            // Write to the file
            outputFile.write(buffer, bytesRead);
        }
        if (bytesRead == -1) {
            close(outputPipe[0]);
            outputFile.close();
            throw std::runtime_error("Failed to read from CGI stdout: " + string(strerror(errno)));
        }

        outputFile.close(); // Close the file after writing
        std::cout << GREEN << "CGI completed. " << totalBytes << " bytes received from " << basename(cgiScriptPath.c_str()) << RESET << std::endl;
        
        close(outputPipe[0]);  // Close read end after reading

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                std::cerr << "CGI script exited with non-zero status: " << exitCode << std::endl;
            }
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