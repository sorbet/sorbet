#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --silence-dev-message test/cli/missing-else-nil-loc/missing_else_nil_loc.rb 2>&1; then
  echo "expected to fail!"
fi
