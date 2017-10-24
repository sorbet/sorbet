#!/bin/bash

# A pretend Python dictionary with bash 3
entry_points=(
    "cfg:main/ruby-typer --print cfg"
    "parse-tree:main/ruby-typer --print parse-tree"
    "ast:main/ruby-typer --print ast"
    "name-table:main/ruby-typer --print name-table"
    "name-tree:main/ruby-typer --print name-tree"
)

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
                echo "$executable $src > $candidate"
                bazel-bin/$executable "$src" 2> /dev/null | grep -v '\[console\] \[error\]' > "$candidate"
            fi
    done
done
