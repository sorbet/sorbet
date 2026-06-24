#!/usr/bin/env bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/and-and-always-truthy/test.rb 2>&1
