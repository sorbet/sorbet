#!/bin/bash

set -euo pipefail
main/sorbet --censor-for-snapshot-tests --silence-dev-message @test/cli/at/at.input 2>&1

if main/sorbet --censor-for-snapshot-tests --silence-dev-message @does_not_exist 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
