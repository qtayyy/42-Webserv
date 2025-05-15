#!/bin/bash

clear

chmod +rwx public/volatile/*
rm -rf public/volatile/*
SOURCE_DIR="source_files"
DEST_DIR="public/volatile"
cp -r "$SOURCE_DIR"/* "$DEST_DIR"
find "$DEST_DIR" -name "*no_perms*" -exec chmod -rwx {} +

make

# Pass the first parameter to ./webserv
./webserv $1