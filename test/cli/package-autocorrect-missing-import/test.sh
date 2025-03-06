#!/bin/bash

cwd="$(pwd)"

tmp="$(mktemp -d)"
cd test/cli/package-autocorrect-missing-import || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --stripe-packages --packager-layers=lib,app --max-threads=0 other use_other_package 2>&1

cat use_other_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --stripe-packages --packager-layers=lib,app --max-threads=0 app_package use_app_package 2>&1

cat use_app_package/__package.rb

echo
echo --------------------------------------------------------------------------
echo

"$cwd/main/sorbet" -a --censor-for-snapshot-tests --silence-dev-message --stripe-packages --packager-layers=lib,app --max-threads=0  false_and_app_package false_package foo 2>&1

cat foo/__package.rb

rm -rf "$tmp"

exit 1
