#!/bin/bash

set -euo pipefail

trace_json="$(mktemp)"
# shellcheck disable=SC2064
trap "rm -f '$trace_json'" EXIT

main/sorbet --silence-dev-message --web-trace-file="$trace_json" \
  test/cli/web-trace-file-non-ascii/web-trace-file-non-ascii.rb 2>&1

if [ -s "$trace_json" ]; then
  echo 'Web trace file exists and has size greater than zero.'
else
  echo 'Web trace file does not exist or has size zero!'
fi
