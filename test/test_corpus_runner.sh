#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
ruby "$rb" > "$rbout" 2>&1

llvmir=$(mktemp -d)
main/sorbet_llvm --silence-dev-message --no-error-count --llvm-ir-folder "$llvmir" "$rb"

bundle="$llvmir/main.bundle"
object="$llvmir/main.o"
external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$object" -undefined dynamic_lookup

srbout=$(mktemp)
# TODO Remove the "$rb" once the bundle does something for real
ruby -r "$llvmir/main.bundle" "$rb" 2>&1 | tee "$srbout"

diff -a "$rbout" "$srbout"

cat "$llvmir/main.ll"
