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
    no_error = False
    while "--no-error" in args:
        args.remove("--no-error")
        no_error = True

    files_index = [i for i, v in enumerate(args) if v == '--']
    if len(files_index) == 0:
        return no_error, args, []
    elif len(files_index) == 1:
        return no_error, args[:files_index[0]], args[files_index[0]+1:]
    else:
        sys.stderr.write("Found more than one instance of --")
        exit(10)

def main():
    args = sys.argv[1:]

    default_ret = get_iwyu_default_return()

    no_error, iwyu_args, files = split_args(args)

    for file in files:

        cmd = ["iwyu"] + iwyu_args + [file]
        p = subprocess.Popen(cmd)
        p.communicate()

        if no_error == False:
            if (p.returncode != default_ret):
                exit(p.returncode)

main()