#!/bin/bash



# disable logs so that log files are not created
chmod -rwx logs/cgi logs/requests logs/responses

output_dir="tests/siege_tests"

run_siege_test() {
    local connections=$1
    local duration=$2
    local timestamp=$(date +"siege_test_%I:%M%p_%d-%m-%Y")
    siege -c "$connections" -t "$duration" -b http://localhost:8080/volatile/empty.txt > "$output_dir/$timestamp"
}

./webserv &
SERVER_PID=$!
sleep 2

# Run siege tests with different parameters
run_siege_test 255 30s
run_siege_test 100 1m

kill $SERVER_PID
chmod +rwx logs/cgi logs/requests logs/responses
