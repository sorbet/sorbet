#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages-missing-export || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --stripe-packages --gen-packages -a . 2>&1

cat A/__package.rb
