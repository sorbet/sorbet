#!/bin/bash
set -eu

# Makes sure all these options don't crash us
for p in parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw name-table name-table-full name-tree name-tree-raw cfg cfg-raw typed-source; do
    echo "--- $p start ---"
    main/ruby-typer -p "$p" -e '1'
    echo "--- $p end ---"
done

for p in init parser desugarer dsl namer cfg inferencer; do
    main/ruby-typer --stop-after $p -e '1'
done
