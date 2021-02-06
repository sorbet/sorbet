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
llvmir="${llvmir:-}"

while getopts ":hdi:" opt; do
  case $opt in
    h)
      usage
      exit 0
      ;;

    d)
      debug=1
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

orig_llvmir=$llvmir
rb_files=( "$@" )

if [ -z "$llvmir" ]; then
  llvmir=$(mktemp -d)
  cleanup() {
    rm -rf "$llvmir"
  }
  trap cleanup EXIT
elif [[ ! -d "$llvmir" ]]; then
  fatal "llvm output directory '${llvmir}' does not exist"
elif [[ "$llvmir" != /* ]]; then
  llvmir="$PWD/$llvmir"
fi

echo
info "Building SorbetLLVM..."
if [ -n "$debug" ]; then
  ./bazel build //main:sorbet --config dbg
else
  ./bazel build //main:sorbet -c opt
fi

if [ -z "$debug" ]; then
  command=( "bazel-bin/main/sorbet" )
else
  command=( "lldb" "--" "./bazel-bin/main/sorbet" )
fi

command=( "${command[@]}" --silence-dev-message "--llvm-ir-folder=$llvmir" \
  "${rb_files[@]}" )

echo
info "Running SorbetLLVM to generate LLVM + shared object..."
info "├─ ${command[*]}"

if "${command[@]}"; then
  success "└─ successfully generated LLVM output."

  if [[ -n "${EMIT_SYNCBACK:-}" && -n "$explicit_llvmir" ]]; then
    echo '### BEGIN SYNCBACK ###'
    find "$orig_llvmir" -name '*.ll' -o -name '*.llo'
    echo '### END SYNCBACK ###'
  fi

else
  fatal "└─ compiling to LLVM failed. See above."
fi
