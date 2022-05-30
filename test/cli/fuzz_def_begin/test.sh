#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --silence-dev-message 2>&1 test/cli/fuzz_def_begin/test.rb; then
  echo "Expected to fail"
  exit 1
fi
