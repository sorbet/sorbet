#!/bin/bash

cwd="$(pwd)"
cd test/cli/dedup-input-files || exit 1

# Dedup dummy files
"$cwd/main/sorbet" --silence-dev-message dummy dummy dummy dummy dummy dummy 2>&1
