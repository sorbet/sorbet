#!/usr/bin/env bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/runtime_types/test.rb 2>&1
