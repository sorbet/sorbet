#!/bin/bash
set -eu

rb=${1/--single_test=/}

rbout=$(mktemp)
ruby "$rb" > "$rbout" 2>&1

srbout=$(mktemp)
main/sorbet_llvm --silence-dev-message --no-error-count "$rb" > "$srbout" 2>&1

diff "$rbout" "$srbout"
