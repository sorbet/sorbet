#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-private/autocorrect-private.rb"
cp "$infile" "$tmp"

old_pwd="$(pwd)"
cd "$tmp" || exit 1

"$old_pwd/main/sorbet" --silence-dev-message -a autocorrect-private.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat autocorrect-private.rb

rm autocorrect-private.rb
rm -rf "$tmp"
