import os
import sys
# sys.stdout = open('output2.log', 'w')
pdf_path = "/home/cooper/coreProgram/qi_ter_webserv/public/upload/webserv.pdf"

if not os.path.exists(pdf_path):
    not_found = b"HTTP/1.1 404 Not Found\r\n" \
                b"Content-Type: text/plain\r\n" \
                b"Content-Length: 13\r\n" \
                b"Connection: close\r\n\r\n" \
                b"404 Not Found"
    sys.stdout.buffer.write(not_found)
    sys.stdout.buffer.flush()
    sys.exit()

with open(pdf_path, 'rb') as file:
    file_content = file.read()

headers = b"HTTP/1.1 200 OK\r\n" \
          b"Content-Type: application/pdf\r\n" \
          b"Content-Length: " + str(len(file_content)).encode() + b"\r\n" \
          b"\r\n\r\n"

sys.stdout.buffer.write(headers + file_content)
sys.stdout.buffer.flush()   
