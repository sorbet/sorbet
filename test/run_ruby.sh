#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

usage() {
  cat << EOF
Usage: test/run_ruby.sh [options] file_1.rb <file_2.rb .. file_n.rb>

  -d       Run the ruby interpreter under the debugger [lldb]
  -g       Run the ruby interpreter under the debugger [gdb] (implies -d)
EOF
}

debug=
use_gdb=

while getopts 'hdg' opt; do
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

    *)
      break;
      ;;
  esac
done

shift $((OPTIND - 1))

rb_file=$1
shift

if [ -z "$rb_file" ]; then
  usage
  exit 1
fi

ruby="./bazel-bin/external/sorbet_ruby_2_7_for_compiler/toolchain/bin/ruby"
sorbet_runtime="./gems/sorbet-runtime/lib/sorbet-runtime.rb"

echo
info "Building Ruby..."

if [ -n "$debug" ]; then
  if [ -n "$use_gdb" ]; then
    ./bazel build @sorbet_ruby_2_7_for_compiler//:ruby --config dbg
    command=("gdb" "--args" "${ruby}")
  else
    ./bazel build @sorbet_ruby_2_7_for_compiler//:ruby --config dbg
    command=("lldb" "--" "${ruby}")
  fi
else
  ./bazel build @sorbet_ruby_2_7_for_compiler//:ruby -c opt
  command=( "${ruby}" )
fi

# Use a temp directory for LLVMIR so we don't accidentally pick up changes from
# the environment
llvmir=$(mktemp -d)
cleanup() {
    rm -r "$llvmir"
}
trap cleanup EXIT

command=("${command[@]}" \
  "--disable=gems" \
  "--disable=did_you_mean" \
  -r "rubygems" \
  -r "$sorbet_runtime" \
  -r "./test/patch_require.rb" \
  -e "require './$rb_file'" \
  "$@" \
  )

echo
info "Running compiled Ruby output..."
info "├─ ${command[*]}"

if llvmir="$llvmir" "${command[@]}"; then
  success "└─ done."
else
  fatal "└─ Non-zero exit. See above."
fi
