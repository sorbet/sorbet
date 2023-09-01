#!/usr/bin/env bash

tmp="$(mktemp -d)"
infile="test/cli/suggest_t_must/suggest_t_must.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message -a suggest_t_must.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest_t_must.rb

rm suggest_t_must.rb
rm "$tmp"
