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

echo "Ruby: $rb"
echo "require './test/preamble.rb'; require './$rb';" > "$rbrunfile"
$ruby "$rbrunfile" 2>&1 | tee "$rbout"

main/sorbet --silence-dev-message --no-error-count --typed=true --llvm-ir-folder "$llvmir" "$rb"

for i in "$llvmir"/*.llo; do
    echo "LLVM IR: $i"
done

echo "require './test/preamble.rb';" > "$runfile"

for o in "$llvmir"/*.o; do
    bundle="${o%.rb}.bundle"
    external/llvm_toolchain/bin/ld -bundle -o "$bundle" "$o" -undefined dynamic_lookup -macosx_version_min 10.14 -lSystem
    echo "Bundle: $bundle"
    echo "require '$bundle';" >> "$runfile"
done

if [[ $rb != *"no-run"* ]]; then
    echo "Run Code: $ruby $runfile"
    echo "Run LLDB: lldb $ruby -- $runfile"
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
