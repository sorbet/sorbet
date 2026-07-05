#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/did-you-mean-false/test.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a --did-you-mean=false test.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

cat test.rb
rm test.rb
