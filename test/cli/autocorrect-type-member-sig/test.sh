#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-type-member-sig/autocorrect-type-member-sig.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-type-member-sig.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

cat autocorrect-type-member-sig.rb

rm autocorrect-type-member-sig.rb
rm "$tmp"
