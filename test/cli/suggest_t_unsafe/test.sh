#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/suggest_t_unsafe/suggest_t_unsafe.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message --suggest-unsafe -a suggest_t_unsafe.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make that the autocorrect applied
  cat suggest_t_unsafe.rb

  rm suggest_t_unsafe.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --censor-for-snapshot-tests --silence-dev-message --suggest-unsafe=custom_wrapper -a suggest_t_unsafe.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make that the autocorrect applied
  cat suggest_t_unsafe.rb

  rm suggest_t_unsafe.rb
)

rm -r "$tmp"
