#!/bin/bash
set -eu

rm -rf out fileout
mkdir -p out fileout

# Make sure these options don't change output
for p in parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw index-tree index-tree-raw symbol-table symbol-table-raw name-tree name-tree-raw resolve-tree resolve-tree-raw cfg cfg-raw cfg-json; do
    echo "--- $p start ---"
    main/sorbet --silence-dev-message --censor-for-snapshot-tests -p "$p" -e '1' | tee "out/$p"
    main/sorbet --silence-dev-message --censor-for-snapshot-tests -p "$p:fileout/$p" -e '1' > /dev/null
    echo "--- $p end ---"
done

# Verify that output printed to stdout matches file output
echo "--- checking diff ---"
diff -r out fileout | sort

echo "--- checking crashes ---"
# Makes sure all these options don't crash us
for p in symbol-table-json symbol-table-full symbol-table-full-raw symbol-table-full-json cfg-proto; do
    main/sorbet --silence-dev-message -p "$p" -e '1' > /dev/null
done

for p in init parser desugarer dsl namer resolver cfg inferencer; do
    main/sorbet --silence-dev-message --stop-after $p -e '1'
done
echo "--- checking crashes end ---"
