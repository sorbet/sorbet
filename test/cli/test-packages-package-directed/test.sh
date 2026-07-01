#!/usr/bin/env bash
set -euo pipefail

root="$PWD"

cd test/cli/test-packages-package-directed || ( echo "Failed to cd!"; exit 1 )

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages --experimental-test-packages --experimental-package-directed . 2>&1
