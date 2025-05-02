#!/bin/bash

# Remove all files from the volatile folder
rm -rf public/volatile/*

# Source directory
SOURCE_DIR="source_tests"

# Destination directory
DEST_DIR="public/volatile"

# Copy all files from source to destination
cp -r "$SOURCE_DIR"/* "$DEST_DIR"