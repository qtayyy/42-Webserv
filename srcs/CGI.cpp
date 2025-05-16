#include "CGI.hpp"
#include "HttpException.hpp"

/* CONSTRUCTOR/DESTRUCTOR */

CGIHandler::CGIHandler() {}
CGIHandler::~CGIHandler() {}

void setEnvironmentVariables(stringDict envVars) {
    for (stringDict::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
        if (unsetenv(it->first.c_str()) != 0)
            LogStream::error() << "Failed to unset environment variable: " << it->first << std::endl;
        if (setenv(it->first.c_str(), it->second.c_str(), 1) != 0)
            LogStream::error() << "Failed to set environment variable: " << it->first << " = " << it->second << std::endl;
        else
            LogStream::error() << std::left << YELLOW << std::setw(20) << "\t" + it->first << RESET << it->second << std::endl;
    }
}

/* GETTER/SETTER */

void   CGIHandler::setEnv(string key, string value) { envVars[key] = value; }
string CGIHandler::getEnv(string key)               { return envVars[key]; }

void CGIHandler:: runCGIExecutable(string &cgiScriptPath, string &requestedFilepath) {
    if (endsWith(cgiScriptPath, ".py"))
        execl("/usr/bin/python3", "python3", cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
    else
        execl(cgiScriptPath.c_str(), cgiScriptPath.c_str(), requestedFilepath.c_str(), NULL);
}

string CGIHandler::handleCgi(
        string        &cgiScriptPath, 
        HttpRequest   &request, 
        int           &exitStatus, 
        LocationBlock &locationBlock,
        HttpResponse  &response
    ) {

    (void)response;

    int inputPipe[2];  // Pipe for sending request body (stdin for CGI)
    int outputPipe[2]; // Pipe for capturing CGI output (stdout from CGI)

    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) 
        throw std::runtime_error("pipe failed");

    string data = request.getBody();

    // Set environment variables

    string fullCGIPath = makeAbsPath(cgiScriptPath);

    this->setEnv("REQUEST_METHOD",  request.getMethod());
    this->setEnv("QUERY_STRING",    request.getQueryString());
    this->setEnv("SERVER_PORT",     request.getHost());
    this->setEnv("PATH_INFO",       request.headerGet("path"));
    this->setEnv("PATH_TRANSLATED", fullCGIPath);
    this->setEnv("REQUEST_URI",     fullCGIPath);
    this->setEnv("SCRIPT_NAME",     cgiScriptPath);
    this->setEnv("CONTENT_LENGTH",  to_string(data.length()));
    this->setEnv("CONTENT_TYPE",    request.headerGet("Content-Type")); //fixme here???
    this->setEnv("SERVER_PROTOCOL", "HTTP/1.1");

    // setEnvironmentVariables(this->envVars);
    // system("./ubuntu_cgi_tester");

    LogStream::pending() << "Running CGI " << fullCGIPath <<  "..." << std::endl;
    // LogStream::log()     << "Input PIPE: "    << inputPipe[0] << ", " << inputPipe[1] << " | Output PIPE: " << outputPipe[0] << ", " << outputPipe[1] << std::endl;
    LogStream::pending() << "Bytes to be written to CGI stdin: " << data.length() << std::endl;
    // LogStream::log("cgi_request.txt", std::ios::out | std::ios::trunc) << data << std::endl;

    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes when writing to a closed pipe

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0) {
        // **CHILD PROCESS** (Executes CGI)
        close(inputPipe[1]);  // Close unused write end of input pipe
        close(outputPipe[0]); // Close unused read end of output pipe

        // Redirect CGI stdin to inputPipe[0] (reading request body)
        if (dup2(inputPipe[0], STDIN_FILENO) == -1 || dup2(outputPipe[1], STDOUT_FILENO) == -1) {
            exit(1); perror("Failed to redirect stdin/stdout for CGI");
        }
        
        close(inputPipe[0]);
        close(outputPipe[1]);
        string rootPath = locationBlock.getRoot();
        std::string reroutedPath = response.getReroutedPath();


        /* ENV */
        char **env = new char*[envVars.size() + 1];
        int      i = 0;
        for (stringDict::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
            std::string envVar = it->first + "=" + it->second;
            env[i] = new char[envVar.length() + 1];
            strcpy(env[i], envVar.c_str());
            i++;
        }
        env[i] = NULL;


        /* EXECUTE PROGRAM */
        char *args[4]; 
        if (endsWith(fullCGIPath, ".py")) {
            args[0] = (char *)"python3";
            args[1] = (char *)(fullCGIPath.c_str());
            args[2] = (char *)(reroutedPath.c_str());
            args[3] = NULL;
            execve("/usr/bin/python3", args, env);
        }

        else {
            args[0] = (char *)(fullCGIPath.c_str());
            args[1] = (char *)(reroutedPath.c_str());

            args[2] = NULL;
            execve(fullCGIPath.c_str(), args, env);
        }

        perror(("execve failed: " + fullCGIPath).c_str());
        exit(1);
    }
    else {
        // PARENT PROCESS (Sends input to CGI)
        close(inputPipe[0]);  // Close unused read end of input pipe
        close(outputPipe[1]); // Close unused write end of output pipe

        // Write request body to CGI stdin
        ssize_t bytesWritten = 0;
        ssize_t length = data.length();

        while (bytesWritten < length) {
            ssize_t result = write(inputPipe[1], data.c_str() + bytesWritten, length - bytesWritten);
            if (result == -1) {
                close(inputPipe[1]);
                throw std::runtime_error("Failed to write to CGI stdin: ");
            }
            bytesWritten += result;
        }
        LogStream::success() << "Wrote " << bytesWritten << " bytes to CGI stdin" << std::endl;
        close(inputPipe[1]);  // Close write end to signal EOF

        // Read CGI response from stdout
        std::vector<char> responseBuffer;
        char              buffer[1024];
        ssize_t           bytesRead;
        ssize_t           totalBytes = 0;

        // Open a file to write the CGI output
        while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
            responseBuffer.insert(responseBuffer.end(), buffer, buffer + bytesRead);
            totalBytes += bytesRead;
        }
        if (bytesRead == -1) {
            close(outputPipe[0]);
            throw std::runtime_error("Failed to read from CGI stdout");
        }

        string finalMsg(responseBuffer.begin(), responseBuffer.end());
        LogStream::log(generateLogFileName(string("logs/cgi/"), request.getUid(), "CGI_OUTPUT")) << finalMsg << std::endl;
        LogStream::success() << "CGI completed. " << totalBytes << " bytes received from " << basename(fullCGIPath.c_str()) << std::endl;
        
        close(outputPipe[0]);  // Close read end after reading

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0)
                LogStream::error() << "CGI script exited with non-zero status: " << exitCode << std::endl;
        }

        exitStatus = WEXITSTATUS(status);
        return string(responseBuffer.begin(), responseBuffer.end());  // Now contains CGI output
    }
    return "";
}
