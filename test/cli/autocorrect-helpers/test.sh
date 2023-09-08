#!/usr/bin/env bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-helpers/autocorrect-helpers.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-helpers.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat autocorrect-helpers.rb

rm autocorrect-helpers.rb
rm "$tmp"
