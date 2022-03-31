#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/arity-messages/arity-messages.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a arity-messages.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make that the autocorrect applied
cat arity-messages.rb

rm arity-messages.rb
