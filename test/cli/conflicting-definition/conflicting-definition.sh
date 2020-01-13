#!/bin/bash
set -eu

set +e
main/sorbet --silence-dev-message --stripe-mode \
  test/cli/conflicting-definition/{a,b}.rb 2>&1
set -e

echo "without --stripe-mode"
main/sorbet --silence-dev-message \
  test/cli/conflicting-definition/{a,b}.rb 2>&1
