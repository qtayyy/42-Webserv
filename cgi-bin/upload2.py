import os
import sys

png_path = "/home/cooper/coreProgram/qi_ter_webserv/public/upload/screenshot.png"

if not os.path.exists(png_path):
    not_found = b"HTTP/1.1 404 Not Found\r\n" \
                b"Content-Type: text/plain\r\n" \
                b"Content-Length: 13\r\n" \
                b"Connection: close\r\n\r\n" \
                b"404 Not Found"
    sys.stdout.buffer.write(not_found)
    sys.stdout.buffer.flush()
    sys.exit()

with open(png_path, 'rb') as file:
    file_content = file.read()

headers = (
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: image/png\r\n"
    "Content-Length: {}\r\n"
    "Connection: close\r\n"
    "\r\n"
).format(len(file_content)).encode()

sys.stdout.buffer.write(headers)
sys.stdout.buffer.write(file_content)
sys.stdout.buffer.flush()