#!/bin/bash
set -euo pipefail

common_args=(
  --silence-dev-message
  --quiet
  --no-error-count
  --print=rbs-rewrite-tree
  --stop-after=rbs
  --parser=prism
  test/cli/rbs-print-rewrite-tree/test.rb
)

main/sorbet \
  "${common_args[@]}" \
  --enable-experimental-rbs-comments \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1

echo --------------------------------------------------------------------------

# intentionally not using `--enable-experimental-rbs-comments`
main/sorbet \
  "${common_args[@]}" \
  test/cli/rbs-print-rewrite-tree/test.rb 2>&1
