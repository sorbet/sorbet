#!/usr/bin/env bash

set -euo pipefail

cd test/cli/packager_did_you_mean

if ../../../main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1 ; then
  echo "Expected to fail, but passed!"
  exit 1
fi
