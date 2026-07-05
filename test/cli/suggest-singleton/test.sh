#!/bin/bash

cwd="$(pwd)"
infile="$cwd/test/cli/suggest-singleton/suggest-singleton.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a suggest-singleton.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest-singleton.rb

rm suggest-singleton.rb
