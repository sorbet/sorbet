#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/literal-type-syntax/literal-type-syntax.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a literal-type-syntax.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat literal-type-syntax.rb

rm literal-type-syntax.rb
rm "$tmp"
