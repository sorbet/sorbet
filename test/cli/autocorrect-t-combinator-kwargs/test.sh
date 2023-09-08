#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-t-combinator-kwargs/autocorrect-t-combinator-kwargs.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-t-combinator-kwargs.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat autocorrect-t-combinator-kwargs.rb

rm autocorrect-t-combinator-kwargs.rb
