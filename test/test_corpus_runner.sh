#!/bin/bash
set -exo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
llvmir=$(mktemp -d)
srbout=$(mktemp)

cleanup() {
    rm -r "$llvmir" "$rbout"
}

# trap cleanup EXIT

ruby "$rb" 2>&1 | tee "$rbout"

main/sorbet_llvm --silence-dev-message --no-error-count --typed=true --llvm-ir-folder "$llvmir" "$rb"

base=$(basename "$rb")
bundle="$llvmir/${base%.rb}.bundle"
external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$llvmir"/*.o -undefined dynamic_lookup

for i in "$llvmir"/*.llo; do
    external/llvm_toolchain/bin/opt -analyze "$i"
done

if [[ $rb != *"no-run"* ]]; then
    runfile=$(mktemp)
    echo "require './test/preamble.rb'; require '$bundle';" > "$runfile"
    ruby "$runfile" | tee "$srbout"

    diff -a "$rbout" "$srbout"
fi

for ext in "llo"; do
    exp=${rb%.rb}.$ext.exp
    if [ -f "$exp" ]; then
        diff <(cat "$llvmir/"*".$ext") "$exp";
    fi
done

cleanup
