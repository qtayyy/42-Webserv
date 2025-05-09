#!/bin/bash

# File to read lines from
input_file="test_lines.txt"

# Output directory
output_dir="format_test"

# Check if the file exists
if [[ ! -f "$input_file" ]]; then
    echo "File $input_file not found!"
    exit 1
fi

mkdir -p "$output_dir"

counter=1

generate_config() {
    local line="$1"
    local output_file="$2"
    local content="$3"
    
    {
        echo "server {"
        echo "	listen 8080;"
        echo "	location / {"
        echo "		$content"
        echo "	}"
        echo "}"
    } > "$output_file"
    
    echo -e "---------------------------------------------------------"
    # cat "$output_file"

	echo -e "\033[33m\t\t$content\n\033[0m"

    timeout 0.05 ./webserv "$output_file" | grep -P "\x1b\[31m"
    echo -e "Generated config for $line in \033[34m$output_file\033[0m"
}



while IFS= read -r line; do
    input_tests=(
        "$line;"                        # lone directive
        "$line test;"                   # directive with argument
        "$line 100;"                    # directive with numeric argument
        "$line 100 test;"               # directive with two arguments
        "$line off;"                    # directive with boolean argument
        "$line on;"                     # directive with boolean argument
        "$line test;\n\t\t$line test2;" # directive argument override
    )

    for suffix in "${input_tests[@]}"; do
        content=$(echo -e "$suffix")
        generate_config "$line" "${output_dir}/output_${counter}_$(echo "$suffix" | wc -w).conf" "$content"
    done

    ((counter++))
done < <(cat "$input_file"; echo)
