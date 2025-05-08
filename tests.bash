#!/bin/bash

conf_file="/home/cooper/coreProgram/webserv_proper/webserv1/conf/good/custom.conf"
output_dir="responses"  # Global variable for output directory
log_file="log_trace.log"
file_index=1  # Initialize the file index
RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[34m"
YELLOW="\e[33m"
MAGENTA="\e[35m"
CYAN="\e[36m"
WHITE="\e[37m"
RESET="\e[0m"



mkdir -p "$output_dir"  # Create the output directory if it doesn't exist

rm -rf "$output_dir"/*  # Remove all files from the output directory
mkdir -p "$output_dir"  # Ensure the directory exists

# Open the log file and delete its contents
> "$log_file"

rm -rf public/upload/*

ITALIC="\e[3m"
RESET="\e[0m"

echo -e "PWD:\t${ITALIC}$(pwd)${RESET}"

# Function to call curl and save the response
call_curl_and_save() {
    local curl_command="$1"
    
    local sanitized_command=$(echo "$curl_command" | sed 's|.*http://||' | sed 's/[^a-zA-Z0-9._-]/_/g')  # Extract after "http://" and sanitize for filename
    
    local output_file="$output_dir/${file_index}_${sanitized_command}.txt"
    local request_file="$output_dir/${file_index}_${sanitized_command}_request.txt"
    touch "$output_file"  # Create the output file if it doesn't exist
    touch "$request_file"  # Create the request file if it doesn't exist

    eval curl -s -i --trace-ascii "$request_file" -o "$output_file" $curl_command  # Use eval to execute the curl command properly

    echo -e "Executing: ${ITALIC}curl -s -i --trace-ascii \"$request_file\" -o \"$output_file\" $curl_command${RESET}"

    echo -e "\tResponse saved:\t${ITALIC}$output_file${RESET}"
    echo -e "\tTrace saved:\t${ITALIC}$request_file${RESET}"

    # Check the HTTP status code from the response
    http_status=$(grep -oP '(?<=HTTP/1\.[01] )\d{3}' "$output_file" | head -n 1)
    http_status_message=$(grep -oP '(?<=HTTP/1\.[01] \d{3} ).*' "$output_file" | head -n 1)

    # Define an array of success statuses
    success_statuses=(200 201 202 204 303 302)

    if [[ " ${success_statuses[@]} " =~ " $http_status " ]]; then
        echo -e "\e[32mReceived success status: HTTP $http_status - $http_status_message\e[0m"  # Print in green for success
    else
        echo -e "\e[33mReceived error status: HTTP $http_status - $http_status_message\e[0m"  # Print in orange for failure
    fi

    echo -e "\nsummary:"

        if [[ -f "$log_file" ]]; then
            last_prefix=$(tail -n 1 "$log_file" | grep -oP '^\d+')

            tac "$log_file" | while IFS= read -r line; do
                current_prefix=$(echo "$line" | grep -oP '^\d+')
                if [[ "$current_prefix" != "$last_prefix" ]]; then
                    break
                fi
                echo -e "\t$line"
            done
        else
            echo "Log file '$log_file' not found."
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
            comment="${line#*#\>\>}"  # Extract everything after #>>
            echo -e "\n$((curl_call_count + 1)) --- \e[34m$comment\e[0m ---"  # Print the next line in blue
            call_curl_and_save "$comment"
            ((curl_call_count++))  # Increment the counter
            ((i++))
            line="${lines[i]}"
        done
        continue
    fi

    if [[ $line =~ ^[[:space:]]*#\ ----.*---- ]]; then
        nested_word=$(echo "$line" | grep -oP '(?<=---- ).*?(?= ----)')  # Extract the word between ----
        echo -e "\n\n\n>>\e[35m$nested_word\e[0m<<"  # Print the extracted word in magenta
    fi

    if [[ $line =~ ^[[:space:]]*location ]]; then
        url="${line#*location}"  # Extract everything after location
        url="${url:1:-2}"      # Skip the first character of the extracted part
        url="${url% *}"         # Remove trailing characters after the space
        url="${url:1}"          # Skip the first character of the extracted part
        echo -e "\n$((curl_call_count + 1)) --- \e[36m$url\e[0m ---"  # Print the next line in cyan
        url=http://localhost:8080/"$url"  # Prepend localhost:8080 to the URL
        call_curl_and_save "$url"
        ((curl_call_count++))  # Increment the counter
    fi
done

# Print the total number of curl calls
echo -e "\nTotal curl calls: $curl_call_count"

# echo -e "\n$CYAN---- SERVER LOGS SAVED $log_file ----"

# # Open and read the log file line by line
# if [[ -f "$log_file" ]]; then
#     previous_prefix=""

#     while IFS= read -r line; do
#         # Extract the prefix number
#         prefix_number=$(echo "$line" | grep -oP '^\d+')

#         if [[ "$prefix_number" != "$previous_prefix" ]]; then
#             # If the prefix number has changed, update the previous prefix
#             previous_prefix="$prefix_number"
#             echo ""
#         fi

#         # Determine the color based on the prefix number
#         if ((prefix_number % 2 == 1)); then
#             color=$CYAN
#         else
#             color=$BLUE
#         fi

#         # Print the line with the selected color
#         echo -e "${color}${line}${RESET}"
#     done < "$log_file"
# else
#     echo "Log file '$log_file' not found."
# fi