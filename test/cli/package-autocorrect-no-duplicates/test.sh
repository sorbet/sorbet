#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-autocorrect-no-duplicates || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --stripe-packages --packager-layers=lib,app --max-threads=0 a b c 2>&1

cat a/__package.rb
