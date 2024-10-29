#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"

run_sorbet() {
  main/sorbet --censor-for-snapshot-tests \
    --silence-dev-message \
    --cache-dir "$dir"/cache \
    test/cli/errors-skip-cache/"$1" \
    2>&1 || true
}

run_twice() {
  echo "====first run (cold cache)===="
  run_sorbet "$1"
  echo "====second run (warm cache)===="
  run_sorbet "$1"
}

run_twice anonymous_block_param.rb
run_twice atoi.rb
run_twice attributes.rb
run_twice block_arg.rb
run_twice constant_reassignment.rb
run_twice props.rb
run_twice sclass.rb
run_twice type_members.rb
run_twice unsupported_nodes.rb
