#!/usr/bin/env bash
set -euo pipefail

sorbet="$PWD/main/sorbet"
cd test/cli/focus-packages-file-table

for printer in file-table-json file-table-full-json; do
  echo "--- $printer ---"
  # Keep all loaded files, including upstream files skipped during inference and unselected package declarations.
  # Other's source stays unread and must be omitted. Only show project paths to avoid snapshotting the payload.
  "$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
    --sorbet-packages --experimental-package-directed --focus-packages=Focused \
    --focus-skip-upstream --focus-skip-downstream --print="$printer" . |
    sed -n '/"path": "\.\//p'
  echo "exit: $?"
done
