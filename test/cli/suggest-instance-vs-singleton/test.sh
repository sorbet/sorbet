#!/bin/bash

tmp="$(mktemp -d)"
infile="test/cli/suggest-instance-vs-singleton/suggest-instance-vs-singleton.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a --suggest-unsafe suggest-instance-vs-singleton.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
cat suggest-instance-vs-singleton.rb

rm suggest-instance-vs-singleton.rb

