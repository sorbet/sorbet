#!/usr/bin/env bash

set -euo pipefail

inputs=(test/cli/rparen/rparen_other.rb)

if main/sorbet --print=desugar-tree --max-threads=0 --censor-for-snapshot-tests --silence-dev-message "${inputs[@]}" 2>&1 ; then
  echo 'expected to fail!'
  exit 1
fi
