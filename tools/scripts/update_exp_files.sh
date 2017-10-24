#!/bin/bash

# A pretend Python dictionary with bash 3
entry_points=( "cfg:main/ruby-typer --cfg --stop cfg"
        "parser:main/ruby-typer --parser --stop parser"
        "desugar:main/ruby-typer --desugar --stop desugar"
        "namer:main/ruby-typer --name-table --stop namer" )

bazel build //main:ruby-typer

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
