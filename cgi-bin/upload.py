#!/usr/bin/python

import cgi
import os
import sys
from urllib.parse import unquote
import cgitb
cgitb.enable()

# Manually set the Content-Type header if not set
boundary = "boundary"

# Read content length and file content
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

# Initialize FieldStorage
try:
    form = cgi.FieldStorage()
except Exception as e:
    sys.exit(1)


# Get request method and route
request_method = os.environ.get("REQUEST_METHOD", "").upper()
route = unquote(os.environ.get("PATH_TRANSLATED", ""))

style = '''
body {
    font-family: Arial, sans-serif;
    background-color: #f4f4f4;
    margin: 0;
    padding: 20px;
}
h2 {
    color: #4CAF50;
}
p {
    background: #fff;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
}
'''

raw_success_page = f'''
<html>
<head>
    <style>
        {style}
    </style>
</head>
<body>
    <h2>File Upload Successful</h2>
    <p>File <strong>%filename</strong> uploaded successfully to <strong>%route</strong>.</p>
</body>
</html>
'''

raw_fail_page = f'''
<html>
<head>
    <style>
        {style}
        h2 {{
            color: red;
        }}
    </style>
</head>
<body>
    <h2>File Upload Failed</h2>
    <p>Failed to upload file.</p>
</body>
</html>
'''


# Handle POST request
if request_method == "POST":
    try:
        if "file" in form:
            file_item = form["file"]
            if file_item.file:
                file_path = os.path.join(route, file_item.filename)
                with open(file_path, 'wb') as f:
                    f.write(file_item.file.read())
                print(raw_success_page.replace("%filename", file_item.filename).replace("%route", route))
            else:
                print("No file content.")
        else:
            print("No file uploaded.")
    except Exception as e:
        print(f"Error: {e}")
else:
    print("Invalid request method.")