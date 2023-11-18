#!/bin/bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-useless-must/autocorrect-useless-must.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-useless-must.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect-useless-must.rb

  rm autocorrect-useless-must.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a autocorrect-useless-must.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect-useless-must.rb

  rm autocorrect-useless-must.rb
)

rm -r "$tmp"
