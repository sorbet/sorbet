#!/usr/bin/env bash

set -euo pipefail

main/sorbet --silence-dev-message --trace-parser test/cli/trace_parser.rb 2>&1
