#!/bin/bash

cwd="$(pwd)"

cd test/cli/config-file-recursive || exit 1

# If we made this a file in git, it wouldn't get copied into the bazel sandbox.
echo '.' > other-config
trap 'rm -f other-config' EXIT

# Look ma, no args! (the args are in sorbet/config & ./other-config)
"$cwd/main/sorbet" 2>&1

echo ------------------------------

# CLI args override config file args
"$cwd/main/sorbet" --typed=false 2>&1
