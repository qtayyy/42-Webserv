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
- priotize root or alias? (I would go with alias)


add:
127.0.0.1       qtay.42.fr shechong.42.fr etlim.42.fr
