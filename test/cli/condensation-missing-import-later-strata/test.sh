#!/usr/bin/env bash

cd test/cli/condensation-missing-import-later-strata || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages \
  --experimental-package-directed \
  --max-threads=0 . 2>&1
