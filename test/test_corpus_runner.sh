#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
ruby "$rb" > "$rbout" 2>&1

llvmir=$(mktemp -d)
main/sorbet_llvm --silence-dev-message --no-error-count --llvm-ir-folder "$llvmir" "$rb"

srbout=$(mktemp)
cat "$llvmir/main.ll" > "$srbout"

diff "$rbout" "$srbout"
