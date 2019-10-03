#!/bin/bash
set -exo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
llvmir=$(mktemp -d)
srbout=$(mktemp)

cleanup() {
    rm -r "$llvmir" "$rbout"
}

trap cleanup EXIT

ruby "$rb" > "$rbout" 2>&1

main/sorbet_llvm --silence-dev-message --no-error-count --llvm-ir-folder "$llvmir" "$rb"
ls "$llvmir"
bundle="$llvmir/main.bundle"
requires=()
for objectFile in "$llvmir/"*.o; do
  bundle=${objectFile%.o}.bundle
  external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$objectFile" -undefined dynamic_lookup
  requires+=(-r "$bundle")
done
ls "$llvmir"

# TODO Remove the "$rb" once the bundle does something for real
ruby "${requires[@]}" "$rb" 2>&1 | tee "$srbout"

diff -a "$rbout" "$srbout"

cat "$llvmir/"*.ll
