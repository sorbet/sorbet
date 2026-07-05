#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect/autocorrect.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message -a autocorrect.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat autocorrect.rb

rm autocorrect.rb
