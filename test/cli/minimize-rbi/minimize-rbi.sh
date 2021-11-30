#!/usr/bin/env bash

set -o euo pipefail

if main/sorbet \
  --silence-dev-message \
  --minimize-to-rbi=test/cli/minimize-rbi/unknown.rbi \
  test/cli/minimize-rbi/minimize-rbi.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo --------------------------------------------------------------------------

if main/sorbet \
  --silence-dev-message \
  --minimize-to-rbi=test/cli/minimize-rbi/unknown.rbi \
  --print=minimized-rbi \
  --autocorrect \
  test/cli/minimize-rbi/minimize-rbi.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo --------------------------------------------------------------------------

# We're using temp files because there was some stdout/stderr syncing problems

stderr_log="$(mktemp)"
trap 'rm -rf "$stderr_log"' EXIT

main/sorbet \
  --silence-dev-message \
  --minimize-to-rbi=test/cli/minimize-rbi/unknown.rbi \
  --print=symbol-table \
  --print=minimized-rbi \
  test/cli/minimize-rbi/minimize-rbi.rb 2> "$stderr_log"

cat "$stderr_log"
