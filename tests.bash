#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[34m"
YELLOW="\e[33m"
MAGENTA="\e[35m"
CYAN="\e[36m"
WHITE="\e[37m"
RESET="\e[0m"

conf_file="conf/good/default.conf"
output_dir="tests/curl_responses"  # Global variable for output directory
log_file="log_trace.log"
file_index=1  # Initialize the file index


# COPY FILES FROM SRC TO VOLATILE
chmod +rwx public/volatile/*
rm -rf public/volatile/*
SOURCE_DIR="source_files"
DEST_DIR="public/volatile"
cp -r "$SOURCE_DIR"/* "$DEST_DIR"
find "$DEST_DIR" -name "*no_perms*" -exec chmod -rwx {} +

# Print all lines of the conf file, ignoring lines containing "#>>"
grep -v '^[[:space:]]*#' "$conf_file" | grep -v '^[[:space:]]*$'


success_statuses=(200 201 202 204 303 302)


mkdir -p "$output_dir"  # Create the output directory if it doesn't exist

rm -rf "$output_dir"/*  # Remove all files from the output directory
mkdir -p "$output_dir"  # Ensure the directory exists

# Open the log file and delete its contents
mkdir -p "$(dirname "$log_file")"  # Ensure the directory exists
> "$log_file"  # Truncate the log file

rm -rf public/upload/*

ITALIC="\e[3m"
RESET="\e[0m"

echo -e "PWD:\t${ITALIC}$(pwd)${RESET}"

# Function to call curl and save the response
call_curl_and_save() {
    local curl_command="$1"
    
    local extracted_command=$(echo "$curl_command" | sed 's|.*http://||')  # Extract after "http://"
    extracted_command=$(echo "$curl_command" | sed 's|.*://[^/]*/||')  # Extract starting from the first slash after the domain
    local sanitized_command=$(echo "$extracted_command" | sed 's/[^a-zA-Z0-9._-]/_/g')  # Sanitize for filename
    
    local output_file="$output_dir/${file_index}_${sanitized_command}.log"
    local request_file="$output_dir/${file_index}_${sanitized_command}_request.log"
    touch "$output_file"  # Create the output file if it doesn't exist
    touch "$request_file"  # Create the request file if it doesn't exist

    eval curl -s -i --trace-ascii "$request_file" -o "$output_file" $curl_command  # Use eval to execute the curl command properly

    if [[ " $@ " =~ " --show_exec " || " $@ " =~ " -x " ]]; then
        echo -e "Executing: ${ITALIC}curl -s -i --trace-ascii \"$request_file\" -o \"$output_file\" $curl_command${RESET}"
    fi

    if [[ " $@ " =~ " --show_saved " || " $@ " =~ " -s " ]]; then
        echo -e "\tResponse saved:\t${ITALIC}$output_file${RESET}"
        echo -e "\tTrace saved:\t${ITALIC}$request_file${RESET}"
    fi

    # Check the HTTP status code from the response
    http_status=$(grep -oP '(?<=HTTP/1\.[01] )\d{3}' "$output_file" | head -n 1)
    http_status_message=$(grep -oP '(?<=HTTP/1\.[01] \d{3} ).*' "$output_file" | head -n 1)

    # Define an array of success statuses

    if [[ " ${success_statuses[@]} " =~ " $http_status " ]]; then
        echo -e "\nReceived: \e[32mHTTP $http_status - $http_status_message\e[0m"  # Print in green for success
    else
        echo -e "\nReceived: \e[33mHTTP $http_status - $http_status_message\e[0m"  # Print in orange for failure
    fi



    if [[ " $@ " =~ " --order " || " $@ " =~ " -r " ]]; then
        echo -e "\nSummary:"

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

    # PRINT CURL COMMAND
    if [[ $line =~ ^.*#\>\> ]]; then
        while [[ $line =~ ^.*#\>\> ]]; do
            comment="${line#*#\>\>}"  # Extract everything after #>>
            echo -e "\n$((curl_call_count + 1)) --- \e[34m$comment\e[0m ---"  # Print the next line in blue
            call_curl_and_save "$comment" "$@"
            ((curl_call_count++))  # Increment the counter
            ((i++))
            line="${lines[i]}"
        done
        continue
    fi

    # PRINT TITLE
    if [[ $line =~ ^[[:space:]]*#\ ----.*---- ]]; then
        nested_word=$(echo "$line" | grep -oP '(?<=---- ).*?(?= ----)')  # Extract the word between ----
        echo -e "\n\n\n>>\e[35m$nested_word\e[0m<<"  # Print the extracted word in magenta
    fi

    # PRINT CURL COMMAND LOCATION BLOCK
    if [[ $line =~ ^[[:space:]]*location ]]; then
        url="${line#*location}"  # Extract everything after location
        url="${url:1:-2}"      # Skip the first character of the extracted part
        url="${url% *}"         # Remove trailing characters after the space
        url="${url:1}"          # Skip the first character of the extracted part
        echo -e "\n$((curl_call_count + 1)) --- \e[36m$url\e[0m ---"  # Print the next line in cyan
        url=http://localhost:8080/"$url"  # Prepend localhost:8080 to the URL
        call_curl_and_save "$url" "$@"
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