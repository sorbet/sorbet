#!/bin/bash
set -eu

# Makes sure all these options don't crash us

if [ $# == 0 ]; then
    binary="main/ruby-typer"
else
    binary=$1
fi

for p in parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw name-table name-table-full name-tree name-tree-raw cfg cfg-raw typed-source; do
    $binary -p $p -e '1'
done

for p in init parser desugarer dsl namer cfg inferencer; do
    $binary --stop-after $p -e '1'
done
