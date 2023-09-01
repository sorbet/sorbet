#!/usr/bin/env bash

cwd="$(pwd)"

tmp="$(mktemp -d)"

cp "$cwd/test/cli/autocorrect-remove-body/autocorrect-remove-body".* "$tmp"

cd "$tmp" || exit 1
if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-remove-body.rb autocorrect-remove-body.rbi 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that the autocorrect applied
echo ----- rb -----
cat autocorrect-remove-body.rb
echo ----- rbi -----
cat autocorrect-remove-body.rbi

rm autocorrect-remove-body.rb
rm autocorrect-remove-body.rbi
