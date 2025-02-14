#!/bin/bash

# Get the content length and type from the environment variables
CONTENT_LENGTH=$CONTENT_LENGTH
CONTENT_TYPE=$CONTENT_TYPE

# Create a temporary file to store the uploaded file
TEMP_FILE=$(mktemp /tmp/upload.XXXXXX)

# Read the body of the POST request from stdin and save it to the temporary file
cat > "$TEMP_FILE"

# Output the HTTP headers
echo "Content-type: text/plain"
echo ""

# Output the details of the uploaded file
echo "File uploaded successfully."
echo "Content Length: $CONTENT_LENGTH"
echo "Content Type: $CONTENT_TYPE"
echo "Stored in: $TEMP_FILE"