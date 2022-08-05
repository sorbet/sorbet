#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/flow-sensitivity-hint/test.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
