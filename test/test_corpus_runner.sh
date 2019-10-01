#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
ruby "$rb" > "$rbout" 2>&1

srbout=$(mktemp)
main/sorbet_llvm --silence-dev-message --no-error-count "$rb" 2>&1 | tee "$srbout"

diff "$rbout" "$srbout"
