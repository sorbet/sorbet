#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.

if ! command -v parallel &> /dev/null; then
  echo "This script requires GNU parallel to be installed"
  exit 1
fi

COMMAND_FILE="$(mktemp)"
trap 'rm -f "$COMMAND_FILE"' EXIT

./bazel build //compiler:sorbet

if [ $# -eq 0 ]; then
  paths=(test/testdata)
else
  paths=("$@")
fi

rb_src=()
while IFS='' read -r line; do
  rb_src+=("$line")
done < <(find "${paths[@]}" -name '*.rb*' | sort)

basename=
srcs=()
exp_extensions="opt.ll ll stderr"

syncback=()

for this_src in "${rb_src[@]}" DUMMY; do
    this_base="${this_src%__*}"
    if [ "$this_base" = "$basename" ]; then
        srcs=("${srcs[@]}" "$this_src")
        continue
    fi

    # Don't update exp files for validate exp tests, as they are hand edited to
    # produce llvm failures
    if [[ "$this_base" =~ "/disabled/validate_exp_test" ]]; then
      continue
    fi

    dir="$(dirname "$this_base")"
    basename="$this_base"
    srcs=("$this_src")

    for ext in $exp_extensions; do
        exp=${basename%.rb}.$ext.exp
        if [ -f "${basename%.rb}.$ext.exp" ]; then
            llvmir=$(mktemp -d)
            syncback+=("$exp")
            echo \
                bazel-bin/compiler/sorbet \
                --silence-dev-message \
                --no-error-count \
                --suppress-non-critical \
                --compiled-out-dir \
                "$llvmir" \
                --llvm-ir-dir \
                "$llvmir" \
                "${srcs[@]}" \
                2\> "$llvmir/update_testdata_exp.stderr"\; \
                \< "$llvmir/$dir/*.$ext" sed -e \'/^target triple =/d\' \> "$exp" \
            >> "$COMMAND_FILE"
        fi
    done
done

if ! parallel --joblog - < "$COMMAND_FILE"; then
  echo 'WARN: parallel exiited non-zero'
fi

if [ "${EMIT_SYNCBACK:-}" != "" ]; then
  echo '### BEGIN SYNCBACK ###'
  for file in "${syncback[@]}"; do
    echo "$file"
  done
  echo '### END SYNCBACK ###'
fi
