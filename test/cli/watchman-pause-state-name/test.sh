#!/bin/bash

set -euo pipefail

if main/sorbet --silence-dev-message --disable-watchman --watchman-pause-state-name=autogen-run -e 'puts 1' 2>&1; then
  echo "expected to fail, but it didn't!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------

main/sorbet --silence-dev-message --watchman-pause-state-name=autogen-run --watchman-pause-state-name=rsync-update -e 'puts 1' 2>&1
