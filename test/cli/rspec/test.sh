#!/bin/bash

set -euo pipefail

# Files are numbered to ensure consistent glob expansion order across platforms
inputs=(test/cli/rspec/rspec{1..4}.rb)

if main/sorbet --max-threads=0 --censor-for-snapshot-tests --silence-dev-message --enable-experimental-requires-ancestor "${inputs[@]}" 2>&1 ; then
  echo "expected to fail!!"
  exit 1
fi
