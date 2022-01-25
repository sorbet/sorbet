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
  parse-tree-whitequark
  parse-tree-json
  desugar-tree
  desugar-tree-raw
  rewrite-tree
  rewrite-tree-raw
  index-tree
  index-tree-raw
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
  cfg-text
  autogen
  document-symbols
  package-tree
  autocorrects
  minimized-rbi
)

./bazel build //main:sorbet //test:print_document_symbols -c opt

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
  # Packager tests are folder based.
  if [[ "$this_src" =~ (.*/packager/([^/]+)/).* ]]; then
    # Basename for all .exp files in packager folder is "pass.$pass.exp"
    this_base="${BASH_REMATCH[1]}pass"
  else
    this_base="${this_src%__*}"
  fi

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
      elif [ "$pass" = "minimized-rbi" ]; then
        args=("--minimize-to-rbi=$basename.minimize.rbi")
      elif [ "$pass" = "package-tree" ]; then
        args=("--stripe-packages")
        extra_prefixes=()
        while IFS='' read -r prefix; do
          extra_prefixes+=("$prefix")
        done < <(grep '# extra-package-files-directory-prefix: ' "${srcs[@]}" | sort | awk -F': ' '{print $2}')
        if [ "${#extra_prefixes[@]}" -gt 0 ]; then
          for prefix in "${extra_prefixes[@]}"; do
            args+=("--extra-package-files-directory-prefix" "${prefix}")
          done
        fi
      fi
      if ! [ -e "$candidate" ]; then
        continue
      fi
      case "$pass" in
        document-symbols)
          echo bazel-bin/test/print_document_symbols \
            "${srcs[@]}" \
            \> "$candidate" \
            2\>/dev/null \
            >>"$COMMAND_FILE"
          ;;
        autocorrects)
          echo tools/scripts/print_autocorrects_exp.sh \
            "${srcs[@]}" \
            \> "$candidate" \
            2\> /dev/null \
            >>"$COMMAND_FILE"
          ;;
        *)
          echo bazel-bin/main/sorbet \
            --silence-dev-message --suppress-non-critical --censor-for-snapshot-tests \
            --print "$pass" --max-threads 0 \
            "${args[@]}" "${srcs[@]}" \
            \> "$candidate" \
            2\>/dev/null \
            >>"$COMMAND_FILE"
          ;;
      esac
    done
  fi

  basename="$this_base"
  srcs=("$this_src")
done

if ! parallel --joblog - < "$COMMAND_FILE"; then
  echo 'WARN: parallel exiited non-zero'
fi
