#!/bin/bash

set -exuo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 1 ]; then
cat <<EOF
usage:
  $0 <crasher>
EOF
exit 1
fi

crasher="$1"

FUZZ_ARG="--stress-incremental-resolver" # to run incremental resolver

mkdir -p fuzz_crashers/fixed/min fuzz_crashers/fixed/original fuzz_crashers/min

file_arg="$(basename "$crasher")"
crash_full_path="$(realpath "$crasher")"
output_file="fuzz_crashers/min/$file_arg"
if [ -f "$output_file.done" ]; then
  echo "already minimized"
  if "bazel-bin/test/fuzz/fuzz_dash_e" "$output_file.done"; then
    echo "already fixed"
    mv "$output_file.done" "fuzz_crashers/fixed/min/$file_arg"
  fi
  exit 0
fi
if [ -f "$output_file" ]; then
  echo "Reusing previous minimized state"
  crash_full_path=$(mktemp)
  cp "$output_file" "$crash_full_path"
fi

if "bazel-bin/test/fuzz/fuzz_dash_e" "$crash_full_path" "$FUZZ_ARG"; then
  echo "already fixed"
  mv "$crash_full_path" "fuzz_crashers/fixed/original/$file_arg"
  exit 0
fi

cancelled=false

handle_INT() {
  kill -INT "$child" 2>/dev/null
  cancelled=true
}

handle_TERM() {
  kill -TERM "$child" 2>/dev/null
  cancelled=true
}

trap handle_INT SIGINT
trap handle_TERM SIGTERM

export PATH="$PATH:$PWD/bazel-sorbet/external/llvm_toolchain/bin"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

# use top 10 frames to tell different errors apart
export ASAN_OPTIONS="dedup_token_length=10"

# start a backgrounded command that we'll monitor
./bazel-bin/test/fuzz/fuzz_dash_e \
  -use_value_profile=1 \
  -dict=test/fuzz/ruby.dict \
  -minimize_crash=1 \
  "$crash_full_path" \
  -exact_artifact_path=fuzz_crashers/min/"$file_arg" \
  "$FUZZ_ARG" \
  &

child=$!
wait "$child"

if "$cancelled"; then
  echo "cancelled"
else
  mv "$output_file" "$output_file.done"
fi
