#!/bin/bash
set -euo pipefail

main/sorbet \
  --print=rbs-rewrite-tree \
  --enable-experimental-rbs-signatures \
  --enable-experimental-rbs-assertions \
  --silence-dev-message \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1

echo --------------------------------------------------------------------------

main/sorbet \
  --print=rbs-rewrite-tree \
  --silence-dev-message \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1
