#!/bin/bash
set -euo pipefail

main/sorbet \
  --print=rbs-rewrite-tree \
  --enable-experimental-rbs-comments \
  --parser=prism \
  --silence-dev-message \
  --no-error-count \
  --quiet \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1

echo --------------------------------------------------------------------------

main/sorbet \
  --print=rbs-rewrite-tree \
  --silence-dev-message \
  --no-error-count \
  --quiet \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1
