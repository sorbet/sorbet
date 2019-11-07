#!/bin/bash
set -eo pipefail

rb=${1/--single_test=/}

llvmir=$(mktemp -d)
rbrunfile=$(mktemp)
rbout=$(mktemp)
srbout=$(mktemp)
srberr=$(mktemp)

cleanup() {
    rm -r "$llvmir" "$rbrunfile" "$rbout" "$srbout" "$srberr"
}

# trap cleanup EXIT

ruby="./external/ruby_2_6_3/ruby"

echo "Source: $rb"
echo "require './run/preamble.rb'; require './$rb';" > "$rbrunfile"
echo "Run Ruby: bazel-bin/$ruby $rbrunfile"
echo "Running Ruby..."
$ruby "$rbrunfile" 2>&1 | tee "$rbout"

echo "Run Sorbet: force_compile=1 llvmir=$llvmir run/ruby $rb"
echo "Temp Dir: $llvmir"
echo "Running Sorbet Compiler..."
run/compile "$llvmir" "$rb"
set +e
force_compile=1 llvmir=$llvmir run/ruby "$rb" 2> "$srberr" | tee "$srbout"
code=$?
set -e

if [ $code -ne 0 ]; then
    stderr="${rb%.rb}.stderr.exp"
    if [ -f "$stderr" ]; then
        diff "${rb%.rb}.stderr.exp" "$srberr"
    else
        cat "$srberr"
        exit $code
    fi
fi

echo "Run LLDB: lldb bazel-bin/$ruby -- $rb"
for i in "$llvmir"/*.llo; do
    echo "LLVM IR: $i"
done
for i in "$llvmir"/*.ll; do
    echo "LLVM IR (unoptimized): $i"
done
for i in "$llvmir"/*.bundle; do
    echo "Bundle: $i"
done

diff -a "$rbout" "$srbout"

for ext in "llo"; do
    exp=${rb%.rb}.$ext.exp
    if [ -f "$exp" ]; then
        actual=("$llvmir/"*".$ext")
        if [ ! -f "${actual[0]}" ]; then
            echo "No LLVMIR found at" "${actual[@]}"
            exit 1
        fi
        diff <(cat "${actual[@]}") "$exp";
    fi
done

cleanup
