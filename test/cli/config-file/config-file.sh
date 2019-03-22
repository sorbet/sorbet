#!/bin/bash

cwd="$(pwd)"

cd test/cli/config-file || exit 1

# Look ma, no args! (the args are in .sorbet/config)
"$cwd/main/sorbet" 2>&1

echo ------------------------------

# CLI args override config file args
"$cwd/main/sorbet" --typed=false 2>&1
