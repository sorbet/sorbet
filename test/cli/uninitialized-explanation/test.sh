#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/uninitialized-explanation/uninitialized.rb 2>&1
