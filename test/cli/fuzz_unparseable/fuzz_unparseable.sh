#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --silence-dev-message -a test/cli/fuzz_unparseable/fuzz_unparseable.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
