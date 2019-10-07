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

bazel build //main:sorbet_llvm

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

for this_src in "${rb_src[@]}" DUMMY; do
  this_base="${this_src%__*}"
  if [ "$this_base" = "$basename" ]; then
    srcs=("${srcs[@]}" "$this_src")
    continue
  fi

  if [ -n "$basename" ]; then
    llvmir=$(mktemp -d)
    echo f\(\) \{ \
      bazel-bin/main/sorbet_llvm --silence-dev-message \
        --no-error-count \
        --llvm-ir-folder \
        "$llvmir" \
        "${srcs[@]}" \
        2\>/dev/null\; \
        for ext in "llo"\; do \
          exp=${basename%.rb}.\$ext.exp\; \
          if [ -f \$exp ]\; then \
            cp "$llvmir"/*.\$ext \$exp\; \
          fi\; \
        done >> "$COMMAND_FILE" \
        \}\; f >> "$COMMAND_FILE"
  fi

  basename="$this_base"
  srcs=("$this_src")
done

parallel --joblog - < "$COMMAND_FILE"
