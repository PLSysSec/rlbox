# iwyu_wrapper

A wrapper around iwyu that

1. Returns 0 exit code on success (Upto version 0.18, iwyu returns 2 as the exit code on success)
2. Allows passing in multiple files to check as args. Invoke `iwyu_wrapper` with same args as `iwyu`, but you can pass in multiple files to check in the form `iwyu_wrapper <iwyu args> -- file1 file2`