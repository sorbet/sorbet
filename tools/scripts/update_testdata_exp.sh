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

passes=(
  parse-tree
  parse-tree-json
  desugar-tree
  desugar-tree-raw
  rewrite-tree
  rewrite-tree-raw
  symbol-table
  symbol-table-raw
  name-tree
  name-tree-raw
  resolve-tree
  resolve-tree-raw
  flatten-tree
  flatten-tree-raw
  ast
  ast-raw
  cfg
  cfg-raw
  cfg-json
  autogen
  document-symbols
)

bazel build //main:sorbet //test:print_document_symbols -c opt

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
    for pass in "${passes[@]}"; do
      candidate="$basename.$pass.exp"
      args=()
      if [ "$pass" = "autogen" ]; then
        args=("--stop-after=namer --skip-rewriter-passes")
      fi
      if ! [ -e "$candidate" ]; then
        continue
      fi
      if [ "$pass" = "document-symbols" ]; then
        echo bazel-bin/test/print_document_symbols \
          "${srcs[@]}" \
          \> "$candidate" \
          2\>/dev/null \
          >>"$COMMAND_FILE"
      else
        echo bazel-bin/main/sorbet \
          --silence-dev-message --suppress-non-critical --censor-for-snapshot-tests \
          --print "$pass" --max-threads 0 \
          "${args[@]}" "${srcs[@]}" \
          \> "$candidate" \
          2\>/dev/null \
          >>"$COMMAND_FILE"
      fi
    done
  fi

  basename="$this_base"
  srcs=("$this_src")
done

parallel --joblog - < "$COMMAND_FILE"
