#!/usr/bin/env python3

from asyncio.subprocess import PIPE
import subprocess
import os
import sys

def format_output(str):
    str = str.decode('UTF-8').strip()
    if " has correct #includes/fwd-decls" in str or str == '':
        return None
    return str

def execute_iwyu(cmd):
    p = subprocess.Popen(["iwyu"] + cmd, stdout=PIPE, stderr=PIPE)
    out, err = p.communicate()

    out = format_output(out)
    err = format_output(err)

    if out:
        print(out)
    if err:
        print(err, file=sys.stderr)

    return p.returncode

def get_iwyu_default_return():
    dir_path = os.path.dirname(os.path.realpath(__file__))
    test_file = os.path.join(dir_path, "test_iwyu.cpp")
    returncode = execute_iwyu([test_file])
    return returncode

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
        returncode = execute_iwyu(iwyu_args + [file])

        if no_error == False:
            if (returncode != default_ret):
                exit(returncode)

main()