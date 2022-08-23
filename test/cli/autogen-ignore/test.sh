#!/bin/bash

set -euo pipefail

# We're using temp files because there was some stdout/stderr syncing problems

stderr_log="$(mktemp)"
trap 'rm -rf "$stderr_log"' EXIT

main/sorbet --silence-dev-message --stop-after=namer -p autogen test/cli/autogen-ignore/autogen-ignore.rb 2> "$stderr_log"

echo --------------------------------------------------------------------------

cat "$stderr_log"
