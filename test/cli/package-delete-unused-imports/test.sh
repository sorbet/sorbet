#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-delete-unused-imports || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --stripe-packages --delete-unused-imports -a . 2>&1

cat C/__package.rb
