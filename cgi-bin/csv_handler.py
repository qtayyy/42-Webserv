import csv
import sys
from cgi import CGIHandler, keys

RED = '\033[91m'
RESET = '\033[0m'

def csv_to_html(filepath):
    try:
        with open(filepath, newline='') as csvfile:
            reader = csv.reader(csvfile)
            html_output = '<table border="1">\n'
            for row in reader:
                html_output += '<tr>\n'
                for column in row:
                    html_output += f'<td>{column}</td>\n'
                html_output += '</tr>\n'
            html_output += '</table>'
            return html_output
    except Exception as e:
        return f"{RED}Error: {e}{RESET}"

import os
import sys

if __name__ == "__main__":
    #move to outer directory

    cgi_handler = CGIHandler()

    if len(sys.argv) != 2:
        print("Usage: python csv_handler.py <csv_filepath>")
    else:
        filepath = cgi_handler.argv[1]

        output = ""
        output += ("<html><head>")
        output += ("<title>Query String</title>")
        output += ("</head><body>")
        output += ("<h2>CSV to HTML</h2>")
        output += (f"<h2>{filepath}</h2>")
        output += (csv_to_html(filepath))
        output += ("</body></html>")

        output = f"Content-Length: {len(output)}\n\n" + output
        print(output)