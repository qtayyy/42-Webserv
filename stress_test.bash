#!/bin/bash

# disable logs so that log files are not created
# chmod -rwx logs/cgi logs/requests logs/responses

output_dir="tests/siege_tests"

run_siege_test() {
    local connections=$1
    local duration=$2
    local timestamp=$(date +"siege_test_%I:%M%p_%d-%m-%Y")
    siege -c $1 -t $2 -b http://localhost:8080/
}

# Run siege tests with different parameters
run_siege_test 255 30s
run_siege_test 100 1m

# chmod +rwx logs/cgi logs/requests logs/responses
