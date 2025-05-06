#!/bin/bash

conf_file="/home/cooper/coreProgram/webserv_proper/webserv1/conf/good/custom.conf"
output_dir="responses"  # Global variable for output directory
file_index=1  # Initialize the file index

mkdir -p "$output_dir"  # Create the output directory if it doesn't exist

rm -rf "$output_dir"/*  # Remove all files from the output directory

# Function to call curl and save the response
call_curl_and_save() {
    local url="$1"
    local sanitized_url=$(echo "$url" | sed 's/[^a-zA-Z0-9._-]/_/g')  # Sanitize URL for filename
    local output_file="$output_dir/${file_index}_${sanitized_url}.txt"
    local request_file="$output_dir/${file_index}_${sanitized_url}_request.txt"

    mkdir -p "$output_dir"  # Ensure the directory exists
    response=$(curl -s -i --trace-ascii "$request_file" -o "$output_file" "$url")  # Save raw request trace to request file, body to output file

    echo "Response body saved to: $output_file"
    echo "Raw request trace saved to: $request_file"

    # Check the HTTP status code from the response
    http_status=$(grep -oP '(?<=HTTP/1\.[01] )\d{3}' "$output_file" | head -n 1)

    if [[ $http_status -ge 200 && $http_status -lt 300 ]]; then
        echo -e "\e[32mSuccess: HTTP $http_status\e[0m"  # Print in green for success
    else
        echo -e "\e[31mFailure: HTTP $http_status\e[0m"  # Print in red for failure
    fi

    ((file_index++))  # Increment the file index
}

# Read lines into an array
mapfile -t lines < "$conf_file"

# Initialize a counter for curl calls
curl_call_count=0

# Iterate over the array using indexing
for ((i = 0; i < ${#lines[@]}; i++)); do
    line="${lines[i]}"
    # Skip the line if it is a comment
    if [[ $line =~ ^[[:space:]]*#\>\> ]]; then
        while [[ $line =~ ^[[:space:]]*#\>\> ]]; do
            comment="${line#*#\>\>}"  # Extract everything after #>>>
            echo -e "\n--- \e[33m$comment\e[0m ---"  # Print the next line in yellow
            call_curl_and_save "$comment"
            ((curl_call_count++))  # Increment the counter
            ((i++))
            line="${lines[i]}"
        done
        continue
    fi

    if [[ $line =~ ^[[:space:]]*location ]]; then
        url="${line#*location}"  # Extract everything after location
        url="${url:1:-2}"      # Skip the first character of the extracted part
        url="${url% *}"         # Remove trailing characters after the space
        url="${url:1}"          # Skip the first character of the extracted part
        echo -e "\n--- \e[36m$url\e[0m ---"  # Print the next line in cyan
        url=http://localhost:8080/"$url"  # Prepend localhost:8080 to the URL
        call_curl_and_save "$url"
        ((curl_call_count++))  # Increment the counter
    fi
done

# Print the total number of curl calls
echo -e "\nTotal curl calls: $curl_call_count"
