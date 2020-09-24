#!/bin/bash
set -eu

rm -rf out fileout
mkdir -p out fileout

# Make sure these options don't change output
for p in parse-tree parse-tree-json parse-tree-whitequark desugar-tree desugar-tree-raw rewrite-tree rewrite-tree-raw index-tree index-tree-raw name-tree name-tree-raw resolve-tree resolve-tree-raw flatten-tree flatten-tree-raw ast ast-raw cfg cfg-raw symbol-table symbol-table-raw; do
    echo "--- $p start ---"
    main/sorbet --silence-dev-message --censor-for-snapshot-tests -p "$p" -e '1' | tee "out/$p"
    main/sorbet --silence-dev-message --censor-for-snapshot-tests -p "$p:fileout/$p" -e '1' > /dev/null
    echo "--- $p end ---"
done

# Verify that output printed to stdout matches file output
echo "--- checking diff ---"
diff -r out fileout | sort

echo "--- checking crashes ---"

# Test --print symbol-table-* options don't crash
for p in symbol-table symbol-table-json symbol-table-proto symbol-table-raw symbol-table-messagepack; do
    main/sorbet --silence-dev-message -p "$p" -e '1' > /dev/null
    main/sorbet --silence-dev-message -p "$p" --print-full -e '1' > /dev/null
done

# Test --print file-table-* options don't crash
for p in file-table-json file-table-proto file-table-messagepack; do
    main/sorbet --silence-dev-message -p "$p" -e '1' > /dev/null
done

# Test --stop-after
for p in init parser desugarer dsl namer resolver cfg inferencer; do
    main/sorbet --silence-dev-message --stop-after $p -e '1'
done

echo "--- checking crashes end ---"
