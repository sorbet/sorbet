#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/call_on_unbounded_type_member/test.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message -a test.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make that the autocorrect applied
  cat test.rb

  rm test.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a test.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make that the autocorrect applied
  cat test.rb

  rm test.rb
)

rm -r "$tmp"
