#!/bin/bash
set -euo pipefail

# Tests both relative and absolute ignore patterns.
main/sorbet --silence-dev-message --censor-for-snapshot-tests --ignore ignore_me.rb --ignore /subfolder --dir test/cli/ignore 2>&1
