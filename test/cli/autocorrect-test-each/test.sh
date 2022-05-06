#!/bin/bash

tmp="$(mktemp)"
infile="test/cli/autocorrect-test-each/each.rb"
cp "$infile" "$tmp"

# Run the autocorrect in place, on the temp file
main/sorbet --silence-dev-message -a "$tmp" 2> /dev/null

# cat the file to see the autocorrected output
cat "$tmp"

echo "=========="

# Verify that we made the right autocorrects for `test_each`
main/sorbet --silence-dev-message "$tmp" 2>&1

rm "$tmp"
