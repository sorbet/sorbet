#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

usage() {
  cat <<EOF
Usage: test/run_sorbet.sh [options] <test_file_1> [<test_file_n> ...]"

  -h       Show this message
  -d       Start sorbet under the debugger [lldb]
  -iPATH   Place build outputs in PATH

  NOTE: when running this script with tools/scripts/remote-script, an explicit
  output path must be provided with -i to enable sync-back.
EOF
}

debug=
if [ -n "${llvmir:-}" ]; then
  explicit_llvmir=1
else
  llvmir=''
fi

if [ -n "${so_folder:-}" ]; then
  explicit_so_folder=1
else
  so_folder=''
fi

while getopts ":hdi:" opt; do
  case $opt in
    h)
      usage
      exit 0
      ;;

    d)
      debug=1
      ;;

    s)
      explicit_so_folder=1
      so_folder="${OPTARG}"
      ;;

    i)
      explicit_llvmir=1
      llvmir="${OPTARG}"
      ;;

    *)
      break
      ;;
  esac
done

shift $((OPTIND - 1))

orig_so_folder="$so_folder"
orig_llvmir="$llvmir"
rb_files=( "$@" )

cleanup() {
  if [ -n "${llvmir:-}" ] && [ -z "${explicit_llvmir:-}" ]; then
    rm -rf "$llvmir"
  fi
  if [ -n "${so_folder:-}" ] && [ -z "${explicit_so_folder:-}" ]; then
    rm -rf "$so_folder"
  fi
}
trap cleanup EXIT

if [ -z "$llvmir" ]; then
  llvmir="$(mktemp -d)"
elif [[ ! -d "$llvmir" ]]; then
  fatal "llvm output directory '${llvmir}' does not exist"
elif [[ "$llvmir" != /* ]]; then
  llvmir="$PWD/$llvmir"
fi

if [ -z "$so_folder" ]; then
  so_folder="$(mktemp -d)"
elif [[ ! -d "$so_folder" ]]; then
  fatal ".so output directory '${so_folder}' does not exist"
elif [[ "$so_folder" != /* ]]; then
  so_folder="$PWD/$so_folder"
fi

echo
info "Building SorbetLLVM..."
if [ -n "$debug" ]; then
  ./bazel build //compiler:sorbet --config dbg --config=static-libs
else
  ./bazel build //compiler:sorbet -c opt
fi

if [ -z "$debug" ]; then
  command=( "bazel-bin/compiler/sorbet" )
else
  command=( "${LLDB:-lldb}" "--" "./bazel-bin/compiler/sorbet" )
fi

command=( "${command[@]}" --silence-dev-message "--so-folder=$so_folder" "--llvm-ir-folder=$llvmir" \
  "${rb_files[@]}" )

echo
info "Running SorbetLLVM to generate LLVM + shared object..."
info "├─ ${command[*]}"

if "${command[@]}"; then
  success "└─ successfully generated LLVM output."

  if [[ -n "${EMIT_SYNCBACK:-}" && -n "$explicit_llvmir" ]]; then
    echo '### BEGIN SYNCBACK ###'
    find "$orig_llvmir" -name '*.ll' -o -name '*.lll' -o -name '*.llo'
    echo '### END SYNCBACK ###'
  fi

else
  fatal "└─ compiling to LLVM failed. See above."
fi
