#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect_method_return/autocorrect_method_return.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message -a autocorrect_method_return.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect_method_return.rb

  rm autocorrect_method_return.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a autocorrect_method_return.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect_method_return.rb

  rm autocorrect_method_return.rb
)

rm -r "$tmp"
