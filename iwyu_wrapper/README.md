# iwyu_wrapper

A wrapper around iwyu that

1. Returns 0 exit code on success (`iwyu` returns `2` as the exit code on success upto version 0.18)
2. Allows passing in multiple files to check as args. Invoke `iwyu_wrapper` with same args as `iwyu`, but you can pass in multiple files to check in the form `iwyu_wrapper <iwyu args> -- file1 file2`
3. Adds a `--no-error` flag to supress all errors and only print suggestions as warnings
