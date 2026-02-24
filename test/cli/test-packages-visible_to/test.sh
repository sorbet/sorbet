#!/usr/bin/env bash

root="$PWD"

cd test/cli/test-packages-visible_to || exit

"$root/main/sorbet" \
  --censor-for-snapshot-tests --silence-dev-message --max-threads=0 \
  --sorbet-packages --experimental-test-packages . 2>&1
