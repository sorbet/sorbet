#!/usr/bin/env bash

tmp="$(mktemp -d)"
infile="test/cli/suggest_t_unsafe/suggest_t_unsafe.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a suggest_t_unsafe.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make that the autocorrect applied
cat suggest_t_unsafe.rb

rm suggest_t_unsafe.rb
rm "$tmp"
