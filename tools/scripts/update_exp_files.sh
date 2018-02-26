#!/bin/bash
set -e

passes=(parse-tree parse-tree-json ast ast-raw dsl-tree dsl-tree-raw name-table name-tree name-tree-raw cfg cfg-raw typed-source)

bazel build //main:ruby-typer -c opt

rb_src=("$@")

if [ -z "${rb_src[*]}" ]; then
    # shellcheck disable=SC2207
    rb_src=(
        $(find ./test/testdata -name '*.rb' | sort)
    )
fi

basename=
srcs=()

for this_src in "${rb_src[@]}" DUMMY; do
    this_base=${this_src%__*}
    if [ "$this_base" = "$basename" ]; then
        srcs=("${srcs[@]}" "$this_src")
        continue
    fi

    if [ "$basename" ]; then
        for pass in "${passes[@]}" ; do
            candidate="$basename.$pass.exp"
            if [ -e "$candidate" ]; then
                echo "bazel-bin/main/ruby-typer --print $pass" "${srcs[@]}" " > $candidate"
                bazel-bin/main/ruby-typer --print "$pass" --max-threads 1 --suppress-non-critical "${srcs[@]}" > "$candidate" 2>/dev/null
            fi
        done
    fi

    basename="$this_base"
    srcs=("$this_src")
done

bazel-bin/main/ruby-typer test/end-to-end-test-input.rb  --configatron-file=test/configatron.yaml 2>&1 | sed -e 's,\(rbi/stdlib.rbi:\)[0-9]*,\1__LINE__,' > test/end-to-end-test.out
bazel-bin/main/ruby-typer -e 'class Foo; end' -p name-table > test/dash-e-test.out 2>&1
test/cli-test.sh bazel-bin/main/ruby-typer > test/cli-test.out
