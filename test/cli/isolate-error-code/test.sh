#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/isolate-error-code/isolate-error-code.rb --isolate-error-code=7017 2>&1
