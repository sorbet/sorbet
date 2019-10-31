#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

rbout=$(mktemp)
llvmir=$(mktemp -d)
rbrunfile=$(mktemp)
runfile=$(mktemp)
srbout=$(mktemp)

cleanup() {
    rm -r "$llvmir" "$rbout" "$rbrunfile" "$runfile" "$srbout"
}

# trap cleanup EXIT

ruby="./external/ruby_2_6_3/ruby"

echo "Source: $rb"
echo "Run Ruby: bazel-bin/$ruby $rb"
echo "Run Sorbet: bin/ruby $rb"
echo "require './bin/preamble.rb'; require './$rb';" > "$rbrunfile"
$ruby "$rbrunfile" 2>&1 | tee "$rbout"

llvmir=$llvmir runfile=$runfile bin/ruby "$rb" | tee "$srbout"

echo "Run LLDB: lldb bazel-bin/$ruby -- $runfile"
for i in "$llvmir"/*.llo; do
    echo "LLVM IR: $i"
done
for i in "$llvmir"/*.ll; do
    echo "LLVM IR (unoptimized): $i"
done
for i in "$llvmir"/*.bundle; do
    echo "Bundle : $i"
done

diff -a "$rbout" "$srbout"

for ext in "llo"; do
    exp=${rb%.rb}.$ext.exp
    if [ -f "$exp" ]; then
        diff <(cat "$llvmir/"*".$ext") "$exp";
    fi
done

cleanup
