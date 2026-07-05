#!/bin/bash

cwd="$(pwd)"

cd test/cli/config-file || exit 1

# Look ma, no args! (the args are in sorbet/config)
"$cwd/main/sorbet" 2>&1

echo ------------------------------

# CLI args override config file args
"$cwd/main/sorbet" --typed=false 2>&1

echo ------------------------------

# Option `--no-config` avoid loading config file
"$cwd/main/sorbet" --silence-dev-message --no-config -e "1 + 1" 2>&1

echo ------------------------------

# Option `--no-config` cannot be used inside a config file
"$cwd/main/sorbet" "@sorbet/bad_no_config" 2>&1

echo ------------------------------

# Option `--no-config` allows another config file
"$cwd/main/sorbet" --no-config "@sorbet/other_config" 2>&1
