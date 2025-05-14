#!/bin/bash

# Start the web server in the background

chmod -rwx logs/cgi logs/requests logs/responses

./webserv &
SERVER_PID=$!

# Wait a moment to ensure the server starts
sleep 2

# Run the siege command
siege -c 50 -t 1m -b http://localhost:8080/volatile/empty.txt

# Terminate the web server after siege ends
kill $SERVER_PID

chmod +rwx logs/cgi logs/requests logs/responses
