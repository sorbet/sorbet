#!/bin/bash
set -euo pipefail

main/sorbet \
  --print=rbs-rewrite-tree \
  --enable-experimental-rbs-comments \
  --silence-dev-message \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1

echo --------------------------------------------------------------------------

main/sorbet \
  --print=rbs-rewrite-tree \
  --silence-dev-message \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1
