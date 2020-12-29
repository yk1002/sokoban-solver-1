#!/usr/bin/python3
"""Pick a single level from an SLC file by ID and print it to stdout.
"""

import argparse
from xml.etree import cElementTree as ET

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('file', nargs=1, help='SLC file path')
    parser.add_argument('-i', '--id', type=str, help='ID of the level to print')
    parser.add_argument('-l', '--list', action='store_true', help='List all level IDs')
    args = parser.parse_args()

    root = ET.parse(args.file[0]).getroot()

    if args.list:
        for level in root.iter('Level'):
            print(level.attrib['Id'])
    elif args.id:
        levels = [level for level in root.iter('Level') if level.attrib['Id'] == args.id]
        if len(levels) == 0:
            raise ValueError(f'The ID "{args.id}" does not match any level.')
        if len(levels) > 1:
            raise ValueError(f'The ID "{args.id}" matches multiple levels.')
        for line in levels[0]:
            print(line.text)
    else:
        parser.print_help()
                    

if __name__ == '__main__':
    main()
