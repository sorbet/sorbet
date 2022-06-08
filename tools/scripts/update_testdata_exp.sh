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

usage() {
  cat <<EOF
$0 [options] [<file>...]

Arguments
  <file>      One or more *.rb files whose exp files to update.
              Defaults to everything in test/testdata/

Options
  --no-build  Don't build anything, just reuse what was already built
  -h, --help  Print this message
EOF
}

BUILD=1
if [ $# -eq 0 ]; then
  paths=(test/testdata)
else
  while true; do
    case $1 in
      --no-build)
        BUILD=
        shift
        ;;
      -h|--help)
        usage
        exit 0
        ;;
      -*)
        1>&2 echo "Unrecognized option '$1'"
        1>&2 usage
        exit 1
        ;;
      *)
        break;
    esac
  done
  paths=("$@")
fi

if [ "$BUILD" != "" ]; then
  ./bazel build //main:sorbet //test:print_document_symbols -c opt
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
    needs_requires_ancestor=false
    if grep -q '^# enable-experimental-requires-ancestor: true' "${srcs[@]}"; then
      needs_requires_ancestor=true
    fi
    for pass in "${passes[@]}"; do
      candidate="$basename.$pass.exp"
      if $needs_requires_ancestor; then
        args=("--enable-experimental-requires-ancestor")
      else
        args=()
      fi
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

# Just update the entire directory, rather than figuring
# out exactly what got updated or didn't.
if [ "${EMIT_SYNCBACK:-}" != "" ]; then
  echo '### BEGIN SYNCBACK ###'
  echo 'test/testdata/'
  echo '### END SYNCBACK ###'
fi
