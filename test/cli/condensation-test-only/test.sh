#!/usr/bin/env bash

cwd="$(pwd)"

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

"$cwd/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages --experimental-package-directed . 2>&1
