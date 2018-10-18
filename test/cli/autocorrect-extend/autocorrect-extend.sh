#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/autocorrect-extend/autocorrect-extend.rb"
cp "$infile" "$tmp"

# Run the autocorrect in place, on the temp file
main/sorbet -a "$infile" 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat "$infile"

# Restore input file (use cat because of symlinks)
cat "$tmp" > "$infile"
rm "$tmp"
