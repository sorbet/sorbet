#!/bin/bash

set -e

cd "$(dirname "$0")"
cd ../..
# we are now at the repo root.

if ! command -v parallel &> /dev/null; then
  echo "This script requires GNU parallel to be installed"
  if [ "$(uname)" == "Darwin" ]; then
      echo "run brew install parallel to install it"
  fi
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
              Defaults to everything in test/testdata/ and test/prism_regression/

Options
  --config=<config>  Build with this Bazel config instead of -c opt.
                     May be repeated.
  --no-build         Don't build anything, just reuse what bazel-bin/
                     points at (whichever config built it last)
  -h, --help         Print this message
EOF
}

BUILD=1
configs=()
while [ $# -gt 0 ]; do
  case $1 in
    --no-build)
      BUILD=
      shift
      ;;
    --config=*)
      configs+=("$1")
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

paths=()
for path in "${@}"; do
  if [[ "$path" = *.exp ]]; then
    # delete shortest match of pattern from back of path
    path=${path%.*.exp}
    if [[ "$path" = */pass ]]; then
      path=${path%/pass}
    fi
  fi

  paths+=("$path")
done

if [ "${#paths[@]}" -eq 0 ]; then
  paths=(test/testdata test/prism_regression)
fi

if [ "${#configs[@]}" -eq 0 ]; then
  # Optimized builds update the whole corpus fastest.
  configs=(-c opt)
fi

if [ "$BUILD" != "" ]; then
  ./bazel build //main:sorbet //test:print_document_symbols "${configs[@]}"
else
  # Without these, every command that writes an exp file would leave it empty.
  for binary in bazel-bin/main/sorbet bazel-bin/test/print_document_symbols; do
    if [ ! -x "$binary" ]; then
      1>&2 echo "$binary is missing. Build it, or omit --no-build"
      exit 1
    fi
  done
fi

rb_src=()
while IFS='' read -r line; do
  rb_src+=("$line")
done < <(find "${paths[@]}" \( -name '*.rb' -o -name '*.rbi' -o -name '*.rbupdate' -o -name '*.rbiupdate' \) | LC_COLLATE=C sort)

basename=
srcs=()

# Whether any file in `srcs` has a line matching the pattern. `srcs` is empty
# for a group made only of update files (`foo.1.rbupdate` has no `__` to group
# it with `foo.rb`), and grep with no files would read stdin, so check first.
srcs_match() {
  [ "${#srcs[@]}" -gt 0 ] && grep -q "$1" "${srcs[@]}"
}

for this_src in "${rb_src[@]}" DUMMY; do
  if grep -q "${this_src#test/}" test/BUILD; then
    # If the test is directly mentioned in test/BULID, it's probably mentioned
    # in an `exclude` line. Let's skip updating.
    continue
  fi

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
    all_srcs=("${srcs[@]}")
    srcs=()
    for src in "${all_srcs[@]}"; do
      case "$src" in
        *.rb|*.rbi)
          srcs+=("$src")
          ;;
      esac
    done

    args=()

    case "${srcs[0]}" in
      test/prism_regression/*|test/testdata/rbs/*)
        args+=("--parser=prism")
        ;;
    esac

    if srcs_match '^# enable-packager: true'; then
      args+=("--sorbet-packages")
      if srcs_match '^# enable-package-directed: true'; then
        args+=("--experimental-package-directed")
      fi

      extra_underscore_prefixes=()
      while IFS='' read -r prefix; do
        extra_underscore_prefixes+=("$prefix")
      done < <(grep '# extra-package-files-directory-prefix-underscore: ' "${srcs[@]}" | sort | awk -F': ' '{print $2}')
      if [ "${#extra_underscore_prefixes[@]}" -gt 0 ]; then
        for prefix in "${extra_underscore_prefixes[@]}"; do
          args+=("--extra-package-files-directory-prefix-underscore" "${prefix}")
        done
      fi

      extra_slash_prefixes_deprecated=()
      while IFS='' read -r prefix; do
        extra_slash_prefixes_deprecated+=("$prefix")
      done < <(grep '# extra-package-files-directory-prefix-slash-deprecated: ' "${srcs[@]}" | sort | awk -F': ' '{print $2}')
      if [ "${#extra_slash_prefixes_deprecated[@]}" -gt 0 ]; then
        for prefix in "${extra_slash_prefixes_deprecated[@]}"; do
          args+=("--extra-package-files-directory-prefix-slash-deprecated" "${prefix}")
        done
      fi

      extra_slash_prefixes=()
      while IFS='' read -r prefix; do
        extra_slash_prefixes+=("$prefix")
      done < <(grep '# extra-package-files-directory-prefix-slash: ' "${srcs[@]}" | sort | awk -F': ' '{print $2}')
      if [ "${#extra_slash_prefixes[@]}" -gt 0 ]; then
        for prefix in "${extra_slash_prefixes[@]}"; do
          args+=("--extra-package-files-directory-prefix-slash" "${prefix}")
        done
      fi
    fi

    if srcs_match '^# enable-experimental-requires-ancestor: true'; then
      args+=("--enable-experimental-requires-ancestor")
    fi

    if srcs_match '^# enable-experimental-rspec: true'; then
      args+=("--enable-experimental-rspec")
    fi

    if srcs_match '^# enable-experimental-rbs-comments: true'; then
      args+=("--enable-experimental-rbs-comments")
    fi

    if srcs_match '^# enable-experimental-method-modifiers: true'; then
      args+=("--enable-experimental-method-modifiers")
    fi

    if srcs_match '^# typed-super: false'; then
      args+=("--typed-super=false")
    fi

    if srcs_match '^# enable-suggest-unsafe: true'; then
      args+=("--suggest-unsafe")
    fi

    for pass in "${passes[@]}"; do
      candidate="$basename.$pass.exp"
      # Document symbols is weird, because it's (currently) the only exp-style
      # test where you can have multiple files and multiple exp files (e.g. one
      # per file, reflecting the document symbols for that particular file).
      # Everything else either has a one-to-one mapping or has a many-to-one
      # mapping (e.g. packager tests).
      #
      # If `candidate` doesn't exist, we might be in the case where we have files
      # foo_test__1.rb, foo_test__2.rb, and so on, `basename`/`this_base` would
      # therefore be `foo_test`, and `foo_test.document-symbols.exp` doesn't exist
      # ...but `foo_test__1.rb.document-symbols.exp` does!
      #
      # So in the case that our first candidate doesn't exist, we need to check
      # for the existence of other possible exp files specifically for
      # document-symbols, and therefore we need an entirely separate list.
      document_symbols_candidates=("$candidate")
      if [ ! -e "$candidate" ]; then
        if [ "$pass" != "document-symbols" ]; then
          continue
        fi

        # Avoid re-checking the thing we already checked.
        if [ "${#srcs[@]}" = 1 ]; then
          continue
        fi

        document_symbols_candidates=()
        for src in "${srcs[@]}"; do
          if [[ "$src" =~ .*.exp ]]; then
            continue
          fi

          src_candidate="$src.document-symbols.exp"
          if [ -e "$src_candidate" ]; then
            document_symbols_candidates=("${document_symbols_candidates[@]}" "$src_candidate")
          fi
        done

        # If we still didn't find anything, we can move on.
        if [ "${#document_symbols_candidates[@]}" = 0 ]; then
          continue
        fi
      fi

      pass_args=()
      if [ "$pass" = "autogen" ]; then
        pass_args=("--stop-after=namer")
      elif [ "$pass" = "minimized-rbi" ]; then
        pass_args=("--minimize-to-rbi=$basename.minimize.rbi")
      fi

      case "$pass" in
        document-symbols)
          # See above for why this case is weird.
          for exp in "${document_symbols_candidates[@]}"; do
            wanted_file="${exp%.document-symbols.exp}"
            # Keep update files in `all_srcs` so that document symbol expectations for them can be regenerated.
            echo bazel-bin/test/print_document_symbols \
              "$wanted_file" "${all_srcs[@]}" \
              \> "$exp" \
              2\>/dev/null \
              >>"$COMMAND_FILE"
          done
          ;;
        autocorrects)
          echo tools/scripts/print_autocorrects_exp.sh \
            "${args[@]+"${args[@]}"}" "${pass_args[@]+"${pass_args[@]}"}" "${srcs[@]}" \
            \> "$candidate" \
            2\> /dev/null \
            >>"$COMMAND_FILE"
          ;;
        *)
          echo bazel-bin/main/sorbet \
            --silence-dev-message --suppress-non-critical --censor-for-snapshot-tests \
            --print "$pass" --max-threads 0 \
            "${args[@]+"${args[@]}"}" "${pass_args[@]+"${pass_args[@]}"}" "${srcs[@]}" \
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
  echo 'test/prism_regression/'
  echo '### END SYNCBACK ###'
fi
