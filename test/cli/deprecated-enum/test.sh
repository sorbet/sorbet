#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/deprecated-enum/deprecated-enum.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a deprecated-enum.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat deprecated-enum.rb

rm deprecated-enum.rb
