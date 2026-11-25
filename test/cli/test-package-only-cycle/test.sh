#!/usr/bin/env bash

cd test/cli/test-package-only-cycle || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --packager-layers --max-threads=0 . 2>&1
