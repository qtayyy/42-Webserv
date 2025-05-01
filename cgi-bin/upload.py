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

#sometimes upload fails, reason being that the content recieved by cgi is empty. Reffer to "cgi_log"

def response():
    debug_response = dedent("""
    HTTP/1.1 200 OK
    Content-Type: text/plain
    Content-Length: 13

    Hello, World!
    """)

    sys.stdout.write(debug_response)
    sys.stdout.flush()

cgitb.enable()

# Initialize FieldStorage
try:
    form = cgi.FieldStorage()
except Exception as e:
    error_message = traceback.format_exc()
    write_to(f"Error: {error_message}")
    sys.exit(1)


try:
    received_data = {key: form.getvalue(key) for key in form.keys()}
    write_to(f"Received data: {received_data}")
except Exception as e:
    error_message = traceback.format_exc()
    write_to(f"Error logging received data: {error_message}")


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
'''

raw_success_page = (
    f"<html>"
    f"<head><style>{style}</style></head>"
    f"<body>"
    f"<h2>File Upload Successful</h2>"
    f"<p>File <strong>%filename</strong> uploaded successfully to <strong>%route</strong>.</p>"
    f"%additional_info"
    f"</body>"
    f"</html>"
)


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
    return generate_response_string(
        content=raw_fail_page,
        status_code=404,
        status_message="Not Found",
        content_type="text/html"
    )

def generate_response_string(
        content: str,
        status_code: int = 404,
        status_message: str = "Not Found",
        content_length: int = None,
        content_type: str = "text/html"):
    
    content = content.strip()  # Remove unintended whitespace or newlines
    return (
        f"HTTP/1.1 {status_code} {status_message}\r\n"
        f"Content-Type: {content_type}\r\n"
        f"Content-Length: {len(content) if content_length is None else content_length}\r\n"
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

# Handle POST request
if request_method == "POST":
    write_to("POST")
    try:
        if "file" in form:
            file_item = form["file"]
            if file_item.file:
                file_path = os.path.join(sys.argv[1], file_item.filename)
                if (os.path.exists(file_path)):
                    error_response = generate_error_page("400 Bad Request", "File already exists.")
                    sys.stdout.write(error_response)
                    sys.stdout.flush()
                    sys.exit(1)
                try:
                    with open(file_path, 'wb') as f:
                        f.write(file_item.file.read())
                except Exception as e:
                    error_message = traceback.format_exc()
                    write_to(f"Error writing file: {error_message}")
                    error_response = generate_error_page("500 Internal Server Error", "An error occurred while saving the file.")
                    sys.stdout.write(error_response)
                    sys.stdout.flush()
                    sys.exit(1)
                file_size = os.path.getsize(file_path)

                response_body = raw_success_page.replace("%additional_info", dedent(f"""
                {generate_env_representation()}
                Program called with arguments: {sys.argv}
                """)).replace("%filename", file_item.filename).replace("%route", file_path).strip()

                response = generate_response_string(
                    content        = response_body,
                    status_code    = 200,
                    status_message = "OK",
                    content_type   = "text/html"
                )
                sys.stdout.write(response)
                sys.stdout.flush()

            else:
                error_response = generate_error_page("400 Bad Request", "No file content.")
                sys.stdout.write(error_response)
                sys.stdout.flush()
        else:
            error_response = generate_error_page("400 Bad Request", "No file was uploaded.")
            sys.stdout.write(error_response)
            sys.stdout.flush()
    except Exception as e:
        error_message = traceback.format_exc()
        write_to(f"Error: {error_message}")
        error_response = generate_error_page("500 Internal Server Error", "An error occurred while processing the request.")
        sys.stdout.write(error_response)
        sys.stdout.flush()

# Handle GET request
elif request_method == "GET":
    write_to("GET")
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
    write_to("UNSUPPORTED METHOD")
# todo post method handled incorrectly, doesnt consider set upload route