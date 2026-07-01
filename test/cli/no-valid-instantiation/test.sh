#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/no-valid-instantiation/no-valid-instantiation.rb 2>&1
