#!/usr/bin/env python3

import subprocess
import os
import sys

def get_iwyu_default_return():
    dir_path = os.path.dirname(os.path.realpath(__file__))
    test_file = os.path.join(dir_path, "test_iwyu.cpp")
    p = subprocess.Popen(["iwyu", test_file])
    p.communicate()
    return p.returncode

def split_args(args):
    files_index = [i for i, v in enumerate(args) if v == '--']
    if len(files_index) == 0:
        return args, []
    elif len(files_index) == 1:
        return args[:files_index[0]], args[files_index[0]+1:]
    else:
        sys.stderr.write("Found more than one instance of --")
        exit(10)

def main():
    args = sys.argv[1:]

    default_ret = get_iwyu_default_return()

    iwyu_args, files = split_args(args)

    for file in files:

        cmd = ["iwyu"] + iwyu_args + [file]
        p = subprocess.Popen(cmd)
        p.communicate()

        if (p.returncode != default_ret):
            exit(p.returncode)

main()