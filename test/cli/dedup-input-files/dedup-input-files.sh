#!/bin/bash

cwd="$(pwd)"
cd test/cli/dedup-input-files || exit 1

# Dedup dummy files
"$cwd/main/sorbet" --silence-dev-message dummy dummy dummy dummy dummy dummy 2>&1

echo ------------------------------

# Dedup real files
"$cwd/main/sorbet" --silence-dev-message foo/stdlib.rbi foo/stdlib.rbi foo/stdlib.rbi 2>&1

echo ------------------------------

# Dedup directories
"$cwd/main/sorbet" --silence-dev-message --no-config foo/ foo/ foo/ 2>&1

echo ------------------------------

# Dedup files from config
"$cwd/main/sorbet" --silence-dev-message 2>&1

echo ------------------------------

# Dedup config files
"$cwd/main/sorbet" --silence-dev-message --no-config @sorbet/config @sorbet/config 2>&1

echo ------------------------------

# Dedup files and keep ordering
"$cwd/main/sorbet" --silence-dev-message --no-config foo/stdlib.rbi foo/foo.rb foo/stdlib.rbi foo/foo.rb 2>&1
