#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-non-final-methods-final-module/autocorrect-non-final-methods-final-module.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1
# Suppress all output because the ordering of errors about non-final methods is
# dependent on hash table iteration order.
if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-non-final-methods-final-module.rb 1>/dev/null 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Cat the file, to make sure that the autocorrect applied
cat autocorrect-non-final-methods-final-module.rb

rm autocorrect-non-final-methods-final-module.rb

rmdir "$tmp"
