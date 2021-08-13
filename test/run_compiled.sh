#!/bin/bash

set -euo pipefail

pushd "$(dirname "$0")/.." > /dev/null

source "test/logging.sh"

debug=
llvmir="${llvmir:-}"

usage() {
  cat << EOF
Usage: test/run_compiled.sh [options] file_1.rb <file_2.rb .. file_n.rb>

  -d       Run the ruby interpreter under the debugger [lldb]
  -iPATH   Store intermediate outputs in PATH

EOF
}

while getopts 'hdi:' opt; do
  case $opt in
    h)
      usage
      exit 0
      ;;

    d)
      debug=1
      ;;

    i)
      llvmir=$OPTARG
      ;;

    *)
      break;
      ;;
  esac
done

shift $((OPTIND - 1))

if [ 1 -gt "$#" ]; then
  echo "Usage: test/run_compiled.sh [-d] <main.rb> [<additional_n.rb> ...]"
  echo
  echo "  NOTE: if the 'llvmir' environment variable is set, that will be used"
  echo "        for compiler output instead."
  exit 1
fi

rb_file=$1
rb_files=( "$@" )

if [ -z "${llvmir:-}" ]; then
  llvmir=$(mktemp -d)
  cleanup() {
    rm -rf "$llvmir"
  }
  trap cleanup EXIT

  # Export llvmir so that run_sorbet picks it up. Real argument parsing in
  # run_sorbet.sh would probably be better.
  export llvmir

  compiled_out_dir=$llvmir
  export compiled_out_dir
elif [[ ! -d "$llvmir" ]]; then
  fatal "llvm output directory '${llvmir}' does not exist"
fi

# ensure that the extension is built
"test/run_sorbet.sh" -s "$llvmir" -i "$llvmir" "${rb_files[@]}"

if [[ "$llvmir" != /* ]]; then
  llvmir="$PWD/$llvmir"
fi

ruby="./bazel-bin/external/sorbet_ruby_2_7/toolchain/bin/ruby"
sorbet_runtime="./gems/sorbet-runtime/lib/sorbet-runtime.rb"

echo
info "Building Ruby..."

if [ -n "$debug" ]; then
  ./bazel build @sorbet_ruby_2_7//:ruby --config dbg --config=static-libs
  command=("${LLDB:-lldb}" "--" "${ruby}")
else
  ./bazel build @sorbet_ruby_2_7//:ruby -c opt
  command=( "${ruby}" )
fi

# Use force_compile to make patch_require.rb fail if the compiled extension
# isn't found.
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
info "├─ llvmir=\"$llvmir\" force_compile=1 ${command[*]}"

if llvmir="$llvmir" force_compile=1 "${command[@]}"; then
  success "└─ done."
else
  fatal "└─ Non-zero exit. See above."
fi
