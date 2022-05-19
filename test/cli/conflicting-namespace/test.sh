#!/bin/bash
set -eu

set +e
main/sorbet --silence-dev-message --stripe-mode --stripe-mode-namespace-collision-check-experimental \
  test/cli/conflicting-namespace/{a,b}.rb 2>&1
set -e

echo "without --stripe-mode-namespace-collision-check-experimental"
main/sorbet --silence-dev-message --stripe-mode \
  test/cli/conflicting-namespace/{a,b}.rb 2>&1
