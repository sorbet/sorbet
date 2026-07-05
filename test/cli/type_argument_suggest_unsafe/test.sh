#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/type_argument_suggest_unsafe/type_argument_suggest_unsafe.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a type_argument_suggest_unsafe.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat type_argument_suggest_unsafe.rb

rm type_argument_suggest_unsafe.rb
