#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

usage() {
  cat <<EOF
Usage: test/run_sorbet.sh [options] <test_file_1> [<test_file_n> ...]"

  -h       Show this message
  -d       Start sorbet under the debugger [lldb]
  -g       Start sorbet under the debugger [gdb] (implies -d)
  -iPATH   Place build outputs in PATH

  NOTE: when running this script with tools/scripts/remote-script, an explicit
  output path must be provided with -i to enable sync-back.
EOF
}

debug=
use_gdb=
if [ -n "${llvmir:-}" ]; then
  explicit_llvmir=1
else
  llvmir=''
fi

if [ -n "${compiled_out_dir:-}" ]; then
  explicit_compiled_out_dir=1
else
  compiled_out_dir=''
fi

while getopts ":hdgs:i:" opt; do
  case $opt in
    h)
      usage
      exit 0
      ;;

    d)
      debug=1
      ;;

    g)
      debug=1
      use_gdb=1
      ;;

    s)
      explicit_compiled_out_dir=1
      compiled_out_dir="${OPTARG}"
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

orig_llvmir="$llvmir"
rb_files=( "$@" )

cleanup() {
  if [ -n "${llvmir:-}" ] && [ -z "${explicit_llvmir:-}" ]; then
    rm -rf "$llvmir"
  fi
  if [ -n "${compiled_out_dir:-}" ] && [ -z "${explicit_compiled_out_dir:-}" ]; then
    rm -rf "compiled_out_dir"
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

if [ -z "$compiled_out_dir" ]; then
  compiled_out_dir="$(mktemp -d)"
elif [[ ! -d "$compiled_out_dir" ]]; then
  fatal ".so output directory '${compiled_out_dir}' does not exist"
elif [[ "$compiled_out_dir" != /* ]]; then
  compiled_out_dir="$PWD/$compiled_out_dir"
fi

echo
info "Building SorbetLLVM..."
if [ -n "$debug" ]; then
  ./bazel build //compiler:sorbet --config dbg --config=static-libs
else
  ./bazel build //compiler:sorbet -c opt
fi

if [ -n "$debug" ]; then
  if [ -n "$use_gdb" ]; then
    command=( "${GDB:-gdb}" "--args" "./bazel-bin/compiler/sorbet" )
  else
    command=( "${LLDB:-lldb}" "--" "./bazel-bin/compiler/sorbet" )
  fi
else
  command=( "bazel-bin/compiler/sorbet" )
fi

command=( "${command[@]}" --silence-dev-message "--compiled-out-dir=$compiled_out_dir" "--llvm-ir-dir=$llvmir" \
  "${rb_files[@]}" )

echo
info "Running SorbetLLVM to generate LLVM + shared object..."
info "├─ ${command[*]}"

if "${command[@]}"; then
  success "└─ successfully generated LLVM output."

  if [[ -n "${EMIT_SYNCBACK:-}" && -n "$explicit_llvmir" ]]; then
    echo '### BEGIN SYNCBACK ###'
    for source in "${rb_files[@]}"; do
      # compgen returns non-zero when the glob doesn't match
      compgen -G "${orig_llvmir}/${source}.*ll" || true
    done
    echo '### END SYNCBACK ###'
  fi

else
  fatal "└─ compiling to LLVM failed. See above."
fi
