#!/usr/bin/python

import cgi
import os
import sys
from urllib.parse import unquote
import cgitb
cgitb.enable()

# Manually set the Content-Type header if not set
boundary = "boundary"
os.environ["CONTENT_TYPE"] = f"multipart/form-data; boundary={boundary}"


# Read content length and file content
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

# file_content = sys.stdin.read(content_length)
# with open("test_post_request", "rb") as f:
#     file_content = f.read()
# print(f"Content Length: {content_length}")
# print(f"File Content: {file_content}")


# Initialize FieldStorage
try:
    form = cgi.FieldStorage()
    print("Form initialized successfully.")
except Exception as e:
    print(f"Error initializing form: {e}")
    sys.exit(1)

# Get request method and route
request_method = os.environ.get("REQUEST_METHOD", "").upper()
route = unquote("." + os.environ.get("PATH_TRANSLATED", ""))
print(f"Request Method: {request_method}")
print(f"Route: {route}")

# Handle POST request
if request_method == "POST":
    try:
        if "file" in form:
            file_item = form["file"]
            if file_item.file:
                file_path = os.path.join(route, file_item.filename)
                with open(file_path, 'wb') as f:
                    f.write(file_item.file.read())
                print(f"File {file_item.filename} uploaded successfully.")
            else:
                print("No file content.")
        else:
            print("No file uploaded.")
    except Exception as e:
        print(f"Error: {e}")
else:
    print("Invalid request method.")