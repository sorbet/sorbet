#!/bin/bash

# Temporary test harness for tests specific to incremental resolver.
#
# Eventually we'll want a way to make normal test_corpus tests work with
# --stress-incremental-resolver.

set -euo pipefail

for file in test/cli/incremental-resolver/*.rb; do
  echo "----- $file ---------------------"
  main/sorbet \
    --silence-dev-message \
    --stress-incremental-resolver \
    "$file" 2>&1
done
