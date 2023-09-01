#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/suggest_unsafe_dead/suggest_unsafe_dead.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a suggest_unsafe_dead.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest_unsafe_dead.rb

rm suggest_unsafe_dead.rb

rm -r "$tmp"
