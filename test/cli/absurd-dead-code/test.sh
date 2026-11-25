#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/absurd-dead-code/test.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
