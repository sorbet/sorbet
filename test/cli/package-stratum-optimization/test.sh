#!/usr/bin/env bash

set -euo pipefail

cd test/cli/package-stratum-optimization

args=(
  --silence-dev-message
  --sorbet-packages
  --experimental-package-directed
  --max-threads=0
  --print=file-table-json
)

# Each package has two files, including __package.rb. The initial strata are:
#   PreludeA, PreludeB (4 files)
#   A, B, C (6 files; A and B form a cycle)
#   D (2 files; imports A and C)
show_strata() {
  ../../../main/sorbet "${args[@]}" "$@" . |
    sed -n -e '/"path":/p' -e '/"pkg_stratum":/p'
}

echo '--- defaults ---'
show_strata

# Split A/B from C, keeping both the cycle and the prelude stratum intact.
echo '--- --stratum-split-threshold=3 ---'
show_strata --stratum-split-threshold=3

# Combine the first two strata to reach the minimum size.
echo '--- --min-stratum-size=5 ---'
show_strata --min-stratum-size=5

# Split first, then combine PreludeA/PreludeB with A/B, and C with D.
echo '--- --stratum-split-threshold=3 --min-stratum-size=5 ---'
show_strata --stratum-split-threshold=3 --min-stratum-size=5
