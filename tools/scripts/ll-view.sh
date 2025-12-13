#!/usr/bin/env bash

set -euo pipefail

green=$'\x1b[0;32m'
cyan=$'\x1b[0;36m'
cnone=$'\x1b[0m'

__use_color=
if [ -t 1 ]; then
  __use_color=1
fi

# Detects whether we can add colors or not
in_color() {
  local color="$1"
  shift

  if [ -z "$__use_color" ]; then
    echo "$*"
  else
    echo "$color$*$cnone"
  fi
}

usage() {
  cat <<EOF

tools/scripts/ll-view.sh: Convert LLVM IR file to graphviz DOT file and SVG

Usage:
  tools/scripts/ll-view.sh <file.ll>

Arguments:
  <file.ll>   The file with the LLVM IR.
EOF
}

# ----- option parsing --------------------------------------------------------

llvm_ir_file=
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      usage
      exit
      ;;
    -*)
      1>&2 echo "error: Unrecoginzed argument: $1"
      1>&1 usage
      exit 1
      ;;
    *)
      if [ "$llvm_ir_file" != "" ]; then
        1>&2 echo "error: Can only pass a single <file.ll> file"
        1>&2 usage
        exit 1
      fi
      llvm_ir_file="$1"
      shift
      ;;
  esac
done

if [ "$llvm_ir_file" = "" ]; then
  1>&2 echo "error: Missing <file.ll>"
  1>&2 usage
  exit 1
fi

if ! command -v realpath &> /dev/null; then
  1>&2 echo "error: ll-view.sh requires GNU realpath"
  exit 1
fi

if ! command -v dot &> /dev/null; then
  1>&2 echo "error: ll-view.sh requires dot (from graphviz)"
  exit 1
fi

project_root="$(realpath "$( dirname "${BASH_SOURCE[0]}" )/../..")"

# ----- main ------------------------------------------------------------------

out_folder="$project_root/ll-view.out"
rm -rf "$out_folder"
mkdir -p "$out_folder"

input_file="$out_folder/input.ll"
cp "$llvm_ir_file" "$input_file"

# We could use bazel run here, but it's more convenient to use whatever happens
# to have been built last, so that we don't have to potentially rebuild LLVM.
llvm_opt="$project_root/bazel-sorbet/external/llvm_toolchain_12_0_0/bin/opt"

if ! [ -x "$llvm_opt" ]; then
  # ... but if nothing was built yet, at least build something.
  bazel build -c opt //tools:opt
fi

(
  cd "$out_folder"
  # /dev/null silences the LLVM bitcode output warning.
  # We could put it in a file, but we don't care about it.
  "$llvm_opt" -dot-cfg "$input_file" > /dev/null
)

i=0
for dot_file in "$out_folder"/.*.dot; do
  dot_basename="$(basename "$dot_file" ".dot")"
  # .foo -> foo
  svg_basename="${dot_basename:1}"
  svg_file="$out_folder/$svg_basename.svg"
  dot -Tsvg "$dot_file" > "$svg_file"
  i=$((i+1))
done

echo
echo "Created $i *.dot and *.svg files in:"
in_color "$cyan" "  ./$(realpath --relative-to "$PWD" "$out_folder")"
in_color "$green" "Done."
