#!/usr/bin/python -u

# Simple parser to create a python dictionary from a yaml file.
# It saves the applications from doing the parsing and
# adding dependencies to additional modules like yaml

import yaml
import pprint

SETTINGS_FILE = 'settings.yaml'
OUTPUT_FILE = 'settings_file.py'
FILE_HEADER = '#!/usr/bin/python -u'

with open(SETTINGS_FILE) as s:
    data = yaml.safe_load(s)

with open(OUTPUT_FILE, 'w') as f:
    f.write(FILE_HEADER)
    f.write('\n')
    f.write('SETTINGS=\\\n')
    pprint.pprint(data, stream=f)
