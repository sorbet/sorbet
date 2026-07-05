#!/usr/bin/env bash

tmp="$(mktemp -d)"
infile="test/cli/class_of-any/class_of-any.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message -a class_of-any.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat class_of-any.rb

rm class_of-any.rb
rm "$tmp"
