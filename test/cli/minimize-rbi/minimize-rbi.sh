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

main/sorbet \
  --silence-dev-message \
  --minimize-to-rbi=test/cli/minimize-rbi/unknown.rbi \
  --print=symbol-table \
  --print=minimized-rbi \
  test/cli/minimize-rbi/minimize-rbi.rb 2>&1
