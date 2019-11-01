#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

llvmir=$(mktemp -d)
rbrunfile=$(mktemp)
srbrunfile=$(mktemp)
rbout=$(mktemp)
srbout=$(mktemp)
srberr=$(mktemp)

cleanup() {
    rm -r "$llvmir" "$rbrunfile" "$srbrunfile" "$rbout" "$srbout" "$srberr"
}

# trap cleanup EXIT

ruby="./external/ruby_2_6_3/ruby"

echo "Source: $rb"
echo "require './run/preamble.rb'; require './$rb';" > "$rbrunfile"
echo "Run Ruby: bazel-bin/$ruby $rbrunfile"
echo "Run Sorbet: run/ruby $rb"
$ruby "$rbrunfile" 2>&1 | tee "$rbout"

echo "Temp Dir: $llvmir"
set +e
llvmir=$llvmir runfile=$srbrunfile run/ruby "$rb" 2> "$srberr" | tee "$srbout"
code=$?
set -e

if [ $code -ne 0 ]; then
    diff "${rb%.rb}.stderr.exp" "$srberr"
fi

echo "Run LLDB: lldb bazel-bin/$ruby -- $srbrunfile"
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
