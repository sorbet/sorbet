#!/bin/bash

set -euo pipefail

common_args=(
  --silence-dev-message
  --quiet
  --no-error-count
  --print=parse-tree
  --stop-after=parser
  -e "123"
)

echo "==== Original parser is the default ===="
main/sorbet "${common_args[@]}" 2>&1

echo "==== '--parser=original' ===="
main/sorbet "${common_args[@]}" --parser=original 2>&1

echo "==== '--parser=prism' ===="
main/sorbet "${common_args[@]}" --parser=prism 2>&1
