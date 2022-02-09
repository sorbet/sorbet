#!/usr/bin/env bash

cwd="$(pwd)"
infile="$cwd/test/cli/max-errors/max-errors.rb"

tmp="$(mktemp -d)"

cp "$infile" "$tmp"

cd "$tmp" || exit 1

echo "------------"
echo "max-errors=3"
echo "------------"

if "$cwd/main/sorbet" --silence-dev-message --max-errors=3 max-errors.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo "------------"
echo "max-errors=0"
echo "------------"

if "$cwd/main/sorbet" --silence-dev-message --max-errors=0 max-errors.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo "-------------"
echo "no max-errors"
echo "-------------"

if "$cwd/main/sorbet" --silence-dev-message max-errors.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
