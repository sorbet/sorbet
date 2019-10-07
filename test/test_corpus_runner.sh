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

ruby "$rb" 2>&1 | tee "$rbout"

main/sorbet_llvm --silence-dev-message --no-error-count --llvm-ir-folder "$llvmir" "$rb"

base=$(basename "$rb")
bundle="$llvmir/${base%.rb}.bundle"
external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$llvmir"/*.o -undefined dynamic_lookup

for ext in "llo"; do
    exp=${rb%.rb}.$ext.exp
    if [ -f "$exp" ]; then
        diff "$llvmir"/*.$ext "$exp";
    fi
done

# TODO Remove the "$rb" once the bundle does something for real
ruby -r "$bundle" "$rb" 2>&1 | tee "$srbout"

diff -a "$rbout" "$srbout"

# cat "$llvmir/"*.ll
