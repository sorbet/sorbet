#!/bin/bash
set -eu

# Make sure these options don't change output
for p in parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw name-table name-tree name-tree-raw cfg cfg-raw typed-source; do
    echo "--- $p start ---"
    main/sorbet -p "$p" -e '1'
    echo "--- $p end ---"
done

# Makes sure all these options don't crash us
for p in name-table-json name-table-full; do
    main/sorbet -p "$p" -e '1' > /dev/null
done

for p in init parser desugarer dsl namer cfg inferencer; do
    main/sorbet --stop-after $p -e '1'
done
