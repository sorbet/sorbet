#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"

common_opts=(
  --silence-dev-message
  --cache-dir "$dir/cache"
  test/cli/typed-super-cache/test.rb
)

echo ----- populate cache: should have no errors -----
main/sorbet "${common_opts[@]}" 2>&1
echo ----- index-tree, with typed super -----
main/sorbet "${common_opts[@]}" --print=index-tree 2>&1
echo ----- index-tree, without typed super -----
main/sorbet "${common_opts[@]}" --typed-super=false --print=index-tree 2>&1
