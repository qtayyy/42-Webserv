# 42-Webserv


**Server capabilities**
- limit_except: <method> ...
- root: <path>
- alias: <path>
- index: <path> ...
- cgi_pass: <cgi_path>
- return: <code> <url>
- autoindex <true/false>
- client_max_body_size <number>
- listen <port>
- server_name <name> ...

# considerations:


add for hostname support:
127.0.0.1       qtay.42.fr shechong.42.fr etlim.42.fr

fuser -k 1234/tcp