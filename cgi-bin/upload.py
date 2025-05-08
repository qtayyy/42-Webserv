#!/usr/bin/python

import cgi
import os
import sys
from urllib.parse import unquote
import cgitb
from textwrap import dedent
import traceback
from datetime import datetime

FILE = "cgi_log"

# Clear the cgi_log file
with open(FILE, 'w') as log_file:
    log_file.truncate(0)

def write_to(msg, filepath:str=FILE):
    with open(filepath, 'a') as f:
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        f.write(f"[{current_time}] {msg}\n")

write_to("cgi called");
for k, v in os.environ.items():
    write_to(f"ENV {k}={v}")

write_to("Received data: {}")

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

try:
    form = cgi.FieldStorage()
except Exception as e:
    error_message = traceback.format_exc()
    write_to(f"Error: {error_message}")
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
h1 {
    color: #4CAF50;
}
table {
    width: 100%;
    border-collapse: collapse;
    border-radius: 10px;
    overflow: hidden;
    padding: 10px;
}
th, td {
    text-align: left;
    background: #fff;
    padding: 10px 5px;
}
a {
    color: #1E90FF;
}
.folder {
    color:rgb(0, 255, 251);
}
.error {
    color: red;
    font-weight: bold;
}
.success {
    color: green;
    font-weight: bold;
}
'''


raw_success_page = (
    f"<html>"
    f"<head><style>{style}</style></head>"
    f"<body>"
    f"<h2 class='success'>File Upload Successful</h2>"
    f"<p>File <strong>%filename</strong> uploaded successfully to <strong>%route</strong>.</p>"
    f"</body>"
    f"</html>"
)


def generate_error_page(
        title, 
        message, 
        error_code=400,
        error_message="Bad Request"):
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
    return generate_response_string(
        content        = raw_fail_page,
        status_code    = error_code,
        status_message = error_message,
        content_type   = "text/html"
    )

def generate_response_string(
        content: str,
        status_code: int    = 404,
        status_message: str = "Not Found",
        content_length: int = None,
        content_type: str   = "text/html",
        **kwargs
    ):
    
    content = content.strip()  # Remove unintended whitespace or newlines
    additional_headers = "".join(f"{key}: {value}\r\n" for key, value in kwargs.items())
    return (
        f"HTTP/1.1 {status_code} {status_message}\r\n"
        f"Content-Type: {content_type}\r\n"
        f"Content-Length: {len(content) if content_length is None else content_length}\r\n"
        f"{additional_headers}"
        f"\r\n"
        f"{content}"
    )


def print_and_write_to(msg, file_path:str=FILE):
    print(msg)
    with open(file_path, 'a') as f:
        f.write(msg)
    return file_path


def generate_env_representation():
    return dedent("""
    <div style="overflow-x:auto;">
        <table border='1' style="width: 100%; table-layout: fixed;">
            <tr><th style="width: 20%;">Key</th><th>Value</th></tr>
            {}
        </table>
    </div>
    """).format("".join(
        f"<tr><td style='word-wrap: break-word; width: 20%;'>{key}</td><td style='word-wrap: break-word;'>{value}</td></tr>"
        for key, value in os.environ.items()
    ))


additional_args = {
    "Pragma": "no-cache",
    "Connection": "close",
    "Cache-Control": "no-store, no-cache, must-revalidate, max-age=0",
}


def bold(text):
    return f"<b>{text}</b>"


def exit_error(generic_msg, error_message, error_code=400):
    error_response = generate_error_page(f"{error_code} {generic_msg}", error_message, error_code, error_message)
    sys.stdout.write(error_response)
    sys.stdout.flush()
    sys.exit(1)


if request_method == "POST":
    write_to("POST")
    if os.environ.get("CONTENT_LENGTH", "") == "0":
        exit_error("Bad Request", "No data received.", 400)

    try:
        if not "file" in form:
            exit_error("Bad Request", "No file was uploaded.", 400)
        
        file_item = form["file"]
        if file_item.file is None: 
            exit_error("Bad Request", "No file content.", 400)
        
        file_path = os.path.join(sys.argv[1], file_item.filename)
        if os.path.exists(file_path): 
            exit_error('Bad Request', f'"{bold(file_item.filename)}" already exists in "{bold(os.path.dirname(file_path))}"', 400)
        
        try:
            with open(file_path, 'wb') as f:
                f.write(file_item.file.read())
        except Exception as e:
            error_message = traceback.format_exc()
            exit_error("Internal Server Error", error_message, 500)
        
        response_body = raw_success_page.\
            replace("%filename", file_item.filename).\
            replace("%route", file_path).\
            strip()

        response = generate_response_string(
            content        = response_body,
            status_code    = 303,
            status_message = "See Other",
            content_type   = "text/html",
            **additional_args
        )
        sys.stdout.write(response)
        sys.stdout.flush()
    
    except Exception as e:
        error_message = traceback.format_exc()
        exit_error("Internal Server Error", error_message, 500)


elif request_method == "GET":
    route = sys.argv[1]
    if not route:
        exit_error("Bad Request", "No file path provided in the request.", 400)

    write_to(f"GET {route}")
    current_path = os.getcwd()
    write_to(f"current_path: {current_path}")
    route = route.lstrip('/')  # Remove leading slash if present
    full_path = os.path.join(current_path, route)
    write_to(f"full_path: {full_path}")

    if os.path.isdir(full_path):
        write_to(f"Directory listing for {full_path}")
        directory_listing = "<ul>"
        for item in os.listdir(full_path):
            item_path = os.path.join(route, item)
            write_to(f"Item: {item}")
            if os.path.isdir(item_path):
                directory_listing += (
                    f"<li class='folder'><b>[DIR]</b> <a data-file='{item}/'>{item}</a></li>"
                )
            else:
                directory_listing += (
                    f"<li><a data-file='{item}'>{item}</a></li>"
                )
        directory_listing += "</ul>"

        # JavaScript to dynamically append href based on current URL
        js_script = """
        <script>
        const basePath = window.location.pathname;
        const normalizedPath = basePath.endsWith('/') ? basePath : basePath + '/';
        document.querySelectorAll('a[data-file]').forEach(link => {
            const fileName = link.getAttribute('data-file');
            link.href = normalizedPath + fileName;
        });
        </script>
        """

        response_body = dedent(f"""
        <html>
        <head><style>{style}</style></head>
        <body>
            <h1>Directory Listing for {route}</h1>
            {directory_listing}
            {js_script}
        </body>
        </html>
        """)

        response = generate_response_string(
            content=response_body,
            status_code=200,
            status_message="OK",
            content_type="text/html",
            **additional_args
        )
        sys.stdout.write(response)
        sys.stdout.flush()
        sys.exit(0)


    extension = os.path.splitext(route)[1]
    content_type = CONTENT_TYPES.get(extension, "Content-Type: text/plain")

    if not os.path.exists(route):
        exit_error("The requested file was not found.", "404 Not Found", 404)

    with open(route, 'rb') as file:
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
    exit_error("Method not allowed.", "405 Method Not Allowed", 405)
# todo post method handled incorrectly, doesnt consider set upload route