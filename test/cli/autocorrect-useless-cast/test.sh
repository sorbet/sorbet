#!/bin/bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect-useless-cast/autocorrect-useless-cast.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-useless-cast.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect-useless-cast.rb

  rm autocorrect-useless-cast.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a autocorrect-useless-cast.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect-useless-cast.rb

  rm autocorrect-useless-cast.rb
)

rm -r "$tmp"
