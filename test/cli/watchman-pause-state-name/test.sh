#!/bin/bash

set -euo pipefail

# The state names only mean something to the watchman subscription, so pairing them with
# --disable-watchman is rejected rather than silently ignored.
if main/sorbet --silence-dev-message --disable-watchman --watchman-pause-state-name=autogen-run -e 'puts 1' 2>&1; then
  echo "expected to fail, but it didn't!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------

# With watchman enabled the flag is accepted, may be repeated, and outside LSP mode has nothing to do.
main/sorbet --silence-dev-message --watchman-pause-state-name=autogen-run --watchman-pause-state-name=rsync-update -e 'puts 1' 2>&1
