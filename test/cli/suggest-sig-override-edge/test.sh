#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/suggest-sig-override-edge/suggest-sig-override-edge.rb"
cp "$infile" "$tmp"

# Run the autocorrect in place, on the temp file
main/sorbet --silence-dev-message -a "$tmp" 2> /dev/null

# cat the file to see the autocorrected output
cat "$tmp"

rm "$tmp"
