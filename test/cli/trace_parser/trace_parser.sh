#!/usr/bin/env bash

set -euo pipefail

tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT

main/sorbet --silence-dev-message --trace-parser test/cli/trace_parser/trace_parser.rb > "$tmp" 2>&1

head -n 4 "$tmp"
