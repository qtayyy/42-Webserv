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

#!/usr/bin/env python3

import sys
import os

# Get the request method from environment variables
request_method = os.environ.get("REQUEST_METHOD", "")

if request_method == "GET":
    pdf_path = r"/home/cooper/coreProgram/qi_ter_webserv/public/upload/2.pdf"


    if not os.path.exists(route):
        error_message_body = "<html><head><title>404 Not Found(CGI-Python)</title></head><body><center><h1>404 Not Found(CGI-Python)</h1></center></body></html>"
        print("HTTP/1.1 404 Not Found(CGI-Python)")
        print("Content-Type: text/html")
        print("Content-Length: " + str(len(error_message_body)))
        print("\r")
        print(error_message_body)
        print("\r")

    root, extension = os.path.splitext(route)
    with open(pdf_path, mode="rb") as file:
        file_content = file.read()
    print("HTTP/1.1 200 OK")
    if extension == ".html":
        print("Content-Type: text/html")
    elif extension == ".css":
        print("Content-Type: text/css")
    elif extension == ".js":
        print("Content-Type: text/javascript")
    elif extension == ".jpg" or extension == ".jpeg":
        print("Content-Type: image/jpeg")
    elif extension == ".png":
        print("Content-Type: image/png")
    elif extension == ".pdf":
        print("Content-Type: application/pdf")
    else:
        print("Content-Type: text/plain")
    print("\r")
    print(file_content)  # Decode the bytes object to a string
    print("\r")

else:
    print("HTTP/1.1 405 Method Not Allowed")
    print("Content-Type: text/html")
    print("\r")
    print("Method not allowed.")
# todo post method handled incorrectly, doesnt consider set upload route