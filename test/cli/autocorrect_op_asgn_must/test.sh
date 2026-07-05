#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/autocorrect_op_asgn_must/autocorrect_op_asgn_must.rb"

tmp="$(mktemp -d)"

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message -a autocorrect_op_asgn_must.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect_op_asgn_must.rb

  rm autocorrect_op_asgn_must.rb
)

echo
echo --------------------------------------------------------------------------
echo

(
  cp "$infile" "$tmp"

  cd "$tmp" || exit 1
  if "$cwd/main/sorbet" --silence-dev-message --suggest-unsafe -a autocorrect_op_asgn_must.rb 2>&1; then
    echo "Expected to fail!"
    exit 1
  fi

  echo
  echo --------------------------------------------------------------------------
  echo

  # Also cat the file, to make sure that the autocorrect applied
  cat autocorrect_op_asgn_must.rb

  rm autocorrect_op_asgn_must.rb
)

rm -r "$tmp"
