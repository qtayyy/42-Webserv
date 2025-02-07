#!/usr/bin/env python3

from cgi import CGIHandler, keys

if __name__ == "__main__":
    cgi_handler = CGIHandler()

    print("<html><head>")
    print("<title>Query String</title>")
    print("</head><body>")
    print("<h2>Query String Parameters</h2>")
    print("<table border='1'>")
    print("<tr><th>Key</th><th>Value</th></tr>")
    for key, value in cgi_handler.query_params.items():
        print(f"<tr><td>{key}</td><td>{value}</td></tr>")
    print("</table>")
    
    print(f"<p>script: {cgi_handler.get(keys.SCRIPT_NAME)}</p>")
    print(f"<p>server name: {cgi_handler.get(keys.SERVER_NAME)}</p>")
    print(f"<p>port: {cgi_handler.get(keys.SERVER_PORT)}</p>")
    print(f"<p>path translated: {cgi_handler.get(keys.PATH_TRANSLATED)}</p>")
    print(f"<p>path info: {cgi_handler.get(keys.PATH_INFO)}</p>")
    print(f"<p>requested resource: {cgi_handler.argv}</p>")
    print("</body></html>")
