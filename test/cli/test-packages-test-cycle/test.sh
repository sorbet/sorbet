#!/usr/bin/env bash

root="$PWD"

cd test/cli/test-packages-test-cycle || ( echo "Failed to cd!"; exit 1 )

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages --experimental-test-packages --experimental-package-directed \
  --packager-layers=lib,app . 2>&1
