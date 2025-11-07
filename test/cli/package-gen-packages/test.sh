#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-gen-packages || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1


"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --gen-packages --packager-layers=util,app -a . 2>&1

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --lsp --stripe-packages --gen-packages --packager-layers=util,app -a . 2>&1

"$cwd/main/sorbet" --max-threads=0 --silence-dev-message --stripe-packages --gen-packages --packager-layers=util,app -a . 2>&1

cat C/__package.rb
