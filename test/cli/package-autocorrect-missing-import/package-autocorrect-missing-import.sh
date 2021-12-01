#!/bin/bash

cwd="$(pwd)"
tmp="$(mktemp -d)"
cd test/cli/package-autocorrect-missing-import || exit 1
for file in $(find . -name '*.rb'); do
    mkdir -p "$tmp/$(dirname $file)"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

"$cwd/main/sorbet" -a --silence-dev-message --stripe-packages --max-threads=0 . 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat __package.rb

rm -rf "$tmp"

exit 1
