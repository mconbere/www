#!/usr/bin/env python2.7
#
# preprocess.py
#
# Convert a literate C program into a text file and a stripped C file.
#
# Dependencies: Requires argparse (default in python2.7)
#
# Input: FILE.c
# Output: FILE.md, FILE.min.c
#
# File specification:
#
# * Any line that begins with `///` will have the initial comment characters removed, as well as up to one
#   space character following the comment characters.
# * Any other line will be indented an additional 4 spaces to turn it into a markdown code block.
#

def change_suffix(name, newsuffix):
    import string
    base, suffix = string.rsplit(name, '.', 1)
    return '.'.join((base, newsuffix))

def main(argv):
    import argparse
    parser = argparse.ArgumentParser(description='Convert a literate C program into a text file and a minial C file.')
    parser.add_argument('file', help='A literate C program file')
    parser.add_argument('--delimiter', default='///', help='The delimiter to indicate a literate line')
    parse_result = parser.parse_args(argv)

    import os.path
    filename = parse_result.file
    fin = open(filename)
    cout = open(change_suffix(filename, 'min.c'), 'w')
    mdout = open(change_suffix(filename, 'md'), 'w')

    delim = parse_result.delimiter
    delim_len = len(delim)

    for line in fin:
        if line[:delim_len] == delim:
            line = line[delim_len:]
            if line[0] == ' ':
                line = line[1:]
            mdout.write(line)
        else:
            if (len(line.strip()) > 0):
                cout.write(line)
            mdout.write('    '+line)

    fin.close()
    cout.close()
    mdout.close()

    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv[1:]))
