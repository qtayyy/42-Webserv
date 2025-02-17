#!/usr/bin/python

import cgi
import os
import sys
from urllib.parse import unquote
import cgitb
from textwrap import dedent


CONTENT_TYPES = {
        ".html": "text/html",
        ".css": "text/css",
        ".js": "text/javascript",
        ".jpg": "image/jpeg",
        ".jpeg": "image/jpeg",
        ".png": "image/png",
        ".pdf": "application/pdf",
    }

cgitb.enable()

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


def generate_error_page(title, message):
    raw_fail_page = dedent(f'''
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
        <h2>{title}</h2>
        <p>{message}</p>
    </body>
    </html>
    ''')
    return raw_fail_page

def generate_response_string(
            content:str,
            status_code:int = 404,
            status_message:str = "Not Found",
            content_length:int = None,
            content_type:str = "text/html"): 
        
        return dedent(f"""
        HTTP/1.1 {status_code} {status_message}
        Content-Type: {content_type}
        Content-Length: {len(content) if content_length is None else content_length}
        
        {content}
        """)

# Handle POST request
if request_method == "POST":
    try:
        if "file" in form:
            file_item = form["file"]
            if file_item.file:
                file_path = os.path.join(route, file_item.filename)
                with open(file_path, 'wb') as f:
                    f.write(file_item.file.read())
                string = generate_response_string(raw_success_page.replace("%filename", file_item.filename).replace("%route", route), status_code=200, status_message="OK")
                print(string)
            else:
                print("No file content.")
        else:
            print(generate_error_page("400 Bad Request", "No file was uploaded."))
    except Exception as e:
        print(f"Error: {e}")

# Handle GET request
elif request_method == "GET":
    pdf_path = route

    extension = os.path.splitext(pdf_path)[1]
    content_type = CONTENT_TYPES.get(extension, "Content-Type: text/plain")

    error_page = generate_error_page("404 Not Found", "The requested file was not found.")

    if not os.path.exists(pdf_path):
        not_found = generate_response_string(error_page)
        sys.stdout.buffer.write(not_found.encode())
        sys.stdout.buffer.flush()
        sys.exit()

    with open(pdf_path, 'rb') as file:
        file_content = file.read()

    headers = (
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: {}\r\n"
        "Content-Length: {}\r\n"
        "Connection: close\r\n"
        "\r\n"
    ).format(content_type, len(file_content)).encode()

    sys.stdout.buffer.write(headers)
    sys.stdout.buffer.write(file_content)
    sys.stdout.buffer.flush()

else:
    print("HTTP/1.1 405 Method Not Allowed")
    print("Content-Type: text/html")
    print("\r")
    print("Method not allowed.")
# todo post method handled incorrectly, doesnt consider set upload route