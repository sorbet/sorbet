#!/bin/bash

cwd="$(pwd)"
tmp="$(mktemp -d)"

cp -R test/cli/package-gen-packages-test-packages-without-flag "$tmp"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --sorbet-packages --gen-packages -a . 2>&1
