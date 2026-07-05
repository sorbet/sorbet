#!/usr/bin/env bash

# This test is the simple case of a single application package, and a single
# test package.

root="$PWD"

cd test/cli/test-packages-minimal || ( echo "Failed to cd!"; exit 1 )

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages --experimental-test-packages . 2>&1
