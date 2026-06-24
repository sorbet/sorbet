#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/no-did-you-mean/no-did-you-mean.rb 2>&1
