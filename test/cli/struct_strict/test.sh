#!/usr/bin/env bash

set -euo pipefail

if main/sorbet --silence-dev-message test/cli/struct_strict/test.rb 2>&1 ; then
  echo "expected to fail!"
  exit 1
fi
