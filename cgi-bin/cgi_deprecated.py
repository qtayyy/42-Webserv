import os
from enum import Enum
import sys

class keys(Enum):
    REQUEST_METHOD = "REQUEST_METHOD"
    QUERY_STRING   = "QUERY_STRING"
    SCRIPT_NAME    = "SCRIPT_NAME"
    SERVER_NAME    = "SERVER_NAME"
    SERVER_PORT    = "SERVER_PORT"
    PATH_INFO      = "PATH_INFO"
    PATH_TRANSLATED= "PATH_TRANSLATED"

class CGIHandler:

    def __init__(self):
        
        self.items = {}
        self.parse_environment_variables()
        self.query_params = self.parse_query_string()
        self.argv = sys.argv

    def parse_environment_variables(self):
        for line in keys:
            self.items[line] = os.environ.get(line.value, '')
        
        if not keys.PATH_INFO in self.items:
            print("Error: PATH_INFO not found in environment variables")
            exit(1)
        
        if  not self.items[keys.PATH_INFO]:
            print("Error: PATH_INFO is empty")
            exit(1)

    def get(self, key: keys):
        return self.items.get(key, '')

    def parse_query_string(self):
        query_params = {}
        if self.items[keys.QUERY_STRING]:
            query_string = self.items[keys.QUERY_STRING]
            parts = query_string.split('&')
            query_params = {key: value for key, value in (part.split('=') for part in parts)}
        
        return query_params

