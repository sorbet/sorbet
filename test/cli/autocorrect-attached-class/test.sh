#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/autocorrect-attached-class/autocorrect-attached-class.rb"
cp "$infile" "$tmp"

old_pwd="$(pwd)"
cd "$tmp" || exit 1

"$old_pwd/main/sorbet" --silence-dev-message -a autocorrect-attached-class.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to verify the resulting file contents.
cat autocorrect-attached-class.rb

rm autocorrect-attached-class.rb
rm -rf "$tmp"
