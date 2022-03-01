#!/usr/bin/env bash

set -euo pipefail

tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT

# We are just testing that the --trace-parser option works.
# The full output is too dependent on the productions in the parser.
main/sorbet --silence-dev-message --trace-parser test/cli/trace_parser/trace_parser.rb > "$tmp" 2>&1

head -n 4 "$tmp"
