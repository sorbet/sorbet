#!/bin/bash

cwd="$(pwd)"
infile="$cwd/test/cli/suggest_static/suggest_static.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a suggest_static.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest_static.rb

rm suggest_static.rb
