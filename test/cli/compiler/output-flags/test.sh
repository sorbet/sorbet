#!/usr/bin/env bash

set -euo pipefail

# Neither IR nor .so output (can't really test that it _hasn't_ output anything anywhere, but this is here for good
# measure).
sorbet --silence-dev-message hello.rb

cleanup() {
  if [ -n "${sodir:-}" ]; then
    rm -rf "$sodir"
  fi

  if [ -n "${irdir:-}" ]; then
    rm -rf "$irdir"
  fi
}

trap cleanup EXIT

echo ".so but no IR output..."

sodir="$(mktemp -d)"
sorbet --silence-dev-message --compiled-out-dir="$sodir" hello.rb

echo "Files in \$sodir:"
pushd "$sodir" >/dev/null
find . -type f -print | sort
popd >/dev/null

rm -rf "$sodir"

echo ".so and IR output..."

sodir="$(mktemp -d)"
irdir="$(mktemp -d)"
sorbet --silence-dev-message --compiled-out-dir="$sodir" --llvm-ir-dir="$irdir" hello.rb

echo "Files in \$sodir:"
pushd "$sodir" >/dev/null
find . -type f -print | sort
popd >/dev/null

echo "Files in \$irdir:"
pushd "$irdir" >/dev/null
find . -type f -print | sort
popd >/dev/null

rm -rf "$sodir"
rm -rf "$irdir"