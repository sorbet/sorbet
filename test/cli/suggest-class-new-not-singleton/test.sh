#!/usr/bin/env bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-class-new-not-singleton/suggest-class-new-not-singleton.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message -a suggest-class-new-not-singleton.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest-class-new-not-singleton.rb

rm suggest-class-new-not-singleton.rb
rm "$tmp"
