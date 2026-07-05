#!/usr/bin/env bash

cwd="$(pwd)"
infiles=(
  "$cwd/test/cli/arity-messages/arity-messages.rb"
  "$cwd/test/cli/arity-messages/super-arity-messages.rb"
)

tmp="$(mktemp -d)"

cp "${infiles[@]}" "$tmp"

cd "$tmp" || exit 1
for infile in *.rb; do
  if "$cwd/main/sorbet" --silence-dev-message -a "$infile" 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat "$infile"

  rm "$infile"

  echo
  echo --------------------------------------------------------------------------
  echo
done
