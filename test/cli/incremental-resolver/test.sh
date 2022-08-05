#!/bin/bash

# Temporary test harness for tests specific to incremental resolver.
#
# Eventually we'll want a way to make normal test_corpus tests work with
# --stress-incremental-resolver.

set -euo pipefail

for file in test/cli/incremental-resolver/*.rb; do
  echo "----- $file ---------------------"
  main/sorbet \
    --censor-for-snapshot-tests --silence-dev-message \
    --stress-incremental-resolver \
    "$file" 2>&1
done

for file in test/cli/incremental-resolver/expect-failures/*.rb; do
  echo "----- $file ---------------------"
  (main/sorbet \
     --censor-for-snapshot-tests --silence-dev-message \
     --stress-incremental-resolver \
     "$file" 2>&1 && echo "Did not find expected failure from $file" && exit 1) || true
done
