#!/bin/bash

clear

# Remove all files from the volatile folder
rm -rf public/volatile/*

# Source directory
SOURCE_DIR="source_tests"

# Destination directory
DEST_DIR="public/volatile"

# Copy all files from source to destination
cp -r "$SOURCE_DIR"/* "$DEST_DIR"

# Remove all permissions for files containing "no_perms" in the destination folder
find "$DEST_DIR" -type f -name "*no_perms*" -exec chmod -rwx {} +

make

# Pass the first parameter to ./webserv
./webserv $1