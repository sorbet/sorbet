#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --silence-dev-message test/cli/flow-sensitivity-hint/test.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
