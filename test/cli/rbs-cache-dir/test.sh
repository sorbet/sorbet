#!/bin/bash
set -euo pipefail

trap 'rm -rf ./my-cache-dir' EXIT


main/sorbet \
  --silence-dev-message \
  --print=index-tree \
  --cache-dir=my-cache-dir \
  --enable-experimental-rbs-comments \
  test/cli/rbs-cache-dir/test.rb 2>&1

echo --------------------------------------------------------------------------

main/sorbet \
  --silence-dev-message \
  --print=index-tree \
  --cache-dir=my-cache-dir \
  test/cli/rbs-cache-dir/test.rb 2>&1
