#!/bin/bash

# A pretend Python dictionary with bash 3
entry_points=( "cfg:cfg/cfg_ast"
        "parser:parser/parse_ast"
        "desugar:ast/desugar/desugar_ast" )

bazel build //ast/desugar:desugar_ast  //parser:parse_ast  //cfg:cfg_ast

rb_src=(
    $(find ./test/testdata/ -name '*.rb')
)

## uncomment the line below to dump format
#clang-format -style=file -dump-config

for src in "${rb_src[@]}"; do
    for entry in "${entry_points[@]}" ; do
        suffix=${entry%%:*}
        executable=${entry#*:}
        candidate="$src.$suffix.exp"
        if [ -e "$candidate" ]
            then
                bazel-bin/$executable "$src" > "$candidate"
            fi
    done
done