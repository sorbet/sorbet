#!/usr/bin/env bash

# Autocorrects are applied by reading and then writing the file on disk after
# typechecking finishes. If the file can't be written (here: it's read-only),
# Sorbet should report an error instead of crashing.

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-unwritable-file/autocorrect-unwritable-file.rb"

tmp="$(mktemp -d)"
chmod 755 "$tmp"
cp "$infile" "$tmp"
chmod 444 "$tmp/autocorrect-unwritable-file.rb"

cd "$tmp" || exit 1

cmd="$cwd/main/sorbet --censor-for-snapshot-tests --silence-dev-message -a autocorrect-unwritable-file.rb"
if [ "$(id -u)" = 0 ]; then
  # root can write to read-only files
  su -s /bin/bash nobody -c "$cmd" 2>&1
else
  $cmd 2>&1
fi
echo "exit code: $?"

echo
echo --------------------------------------------------------------------------
echo

# The file should be unchanged
cat autocorrect-unwritable-file.rb

cd "$cwd" || exit 1
rm -rf "$tmp"
