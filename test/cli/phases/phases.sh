#!/bin/bash
set -eu

# Make sure these options don't change output
for p in parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw symbol-table symbol-table-raw name-tree name-tree-raw resolve-tree resolve-tree-raw file-table-json cfg cfg-raw typed-source; do
    echo "--- $p start ---"
    main/sorbet --silence-dev-message -p "$p" -e '1'
    echo "--- $p end ---"
done

# Makes sure all these options don't crash us
for p in symbol-table-json symbol-table-full symbol-table-full-raw; do
    main/sorbet --silence-dev-message -p "$p" -e '1' > /dev/null
done

for p in init parser desugarer dsl namer resolver cfg inferencer; do
    main/sorbet --silence-dev-message --stop-after $p -e '1'
done
