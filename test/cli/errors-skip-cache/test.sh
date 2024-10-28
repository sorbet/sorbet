#!/bin/bash
set -euo pipefail

dir=$(mktemp -d)
cleanup() {
    rm -r "$dir"
}
trap cleanup EXIT

mkdir "$dir/cache"

run_sorbet() {
  # Ideally this would include counters output so that we could verify the cache
  # miss, but there are a lot of unstable metrics in the counters output, and
  # filtering it means that we'll lose the errors that duplicate across sorbet
  # invocations.
  #
  # TODO(trevor): update this to include counter output if we fix `--counter`
  main/sorbet --censor-for-snapshot-tests \
    --silence-dev-message test/cli/errors-skip-cache/test.rb \
    --cache-dir "$dir"/cache \
    2>&1 || true
}

echo "====first run (cold cache)===="
run_sorbet
echo "====second run (warm cache)===="
run_sorbet
