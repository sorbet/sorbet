#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
llvmir=$(mktemp -d)

cleanup() {
    rm -r "$llvmir" "$rbout"
}

# trap cleanup EXIT

ruby="/Users/$(whoami)/.rbenv/shims/ruby"

rbrunfile=$(mktemp)
echo "Ruby: $rb"
echo "require './test/preamble.rb'; require './$rb';" > "$rbrunfile"
$ruby "$rbrunfile" 2>&1 | tee "$rbout"

main/sorbet_llvm --silence-dev-message --no-error-count --typed=true --llvm-ir-folder "$llvmir" "$rb"

base=$(basename "$rb")
bundle="$llvmir/${base%.rb}.bundle"
external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$llvmir"/*.o -undefined dynamic_lookup -macosx_version_min 10.14 -lSystem
echo "Bundle: $bundle"

for i in "$llvmir"/*.llo; do
    echo "LLVM IR: $i"
    external/llvm_toolchain/bin/opt -analyze "$i"
done

if [[ $rb != *"no-run"* ]]; then
    runfile=$(mktemp)
    srbout=$(mktemp)
    echo "require './test/preamble.rb'; require '$bundle';" > "$runfile"
    echo "Run Code: $ruby $runfile"
    echo "Run LLDB: lldb ./bazel-out/darwin-dbg/bin/external/ruby_2_4_3/bin/ruby -- $runfile"
    $ruby "$runfile" | tee "$srbout"

    diff -a "$rbout" "$srbout"
fi

for ext in "llo"; do
    exp=${rb%.rb}.$ext.exp
    if [ -f "$exp" ]; then
        diff <(cat "$llvmir/"*".$ext") "$exp";
    fi
done

cleanup
