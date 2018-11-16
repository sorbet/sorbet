#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/autocorrect-extend/autocorrect-extend.rb"
cp "$infile" "$tmp"

# Run the autocorrect in place, on the temp file
main/sorbet -a "$tmp" 2> /dev/null

# cat the file to see the autocorrected output
cat "$tmp"

rm "$tmp"
