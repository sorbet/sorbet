#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/super-did-you-mean/test.rb 2>&1
