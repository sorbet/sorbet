#!/bin/bash

set -euo pipefail

cd test/cli/package-attributed-errors

if ../../../main/sorbet --max-threads=0 --censor-for-snapshot-tests --silence-dev-message --sorbet-packages --package-attributed-errors . 2>&1 ; then
  echo "expected to fail!!"
  exit 1
fi
