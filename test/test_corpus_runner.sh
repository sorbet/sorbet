#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
ruby "$rb" > "$rbout" 2>&1

srbout=$(mktemp)
llvmir=$(mktemp -d)
ls "$llvmir"
main/sorbet_llvm --silence-dev-message --no-error-count --llvm-ir-folder "$llvmir" "$rb" 2>&1 | tee "$srbout"

diff "$rbout" "$srbout"
