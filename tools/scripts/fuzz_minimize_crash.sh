#!/bin/bash

set -exuo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 1 ]; then
cat <<EOF
usage:
  $0 <target> <crasher> [<options>]

example target:
  fuzz_dash_e
  fuzz_doc_symbols
  fuzz_hover

example options:
  --stress-incremental-resolver
EOF
exit 1
fi

target="$1"
shift
crasher="$1"
shift

mkdir -p fuzz_crashers/fixed/min fuzz_crashers/fixed/original fuzz_crashers/min

crasher_name="$(basename "$crasher")"
output_file="fuzz_crashers/min/$crasher_name"
done_file="$output_file.done"

if [ -f "$done_file" ]; then
  echo "already minimized"
  if "./bazel-bin/test/fuzz/$target" "$done_file"; then
    echo "already fixed"
    mv "$done_file" "fuzz_crashers/fixed/min/$crasher_name"
  fi
  exit
fi

if [ -f "$output_file" ]; then
  echo "Reusing previous minimized state"
  crasher="$(mktemp)"
  cp "$output_file" "$crasher"
fi

if "./bazel-bin/test/fuzz/$target" "$crasher" "$@"; then
  echo "already fixed"
  mv "$crasher" "fuzz_crashers/fixed/original/$crasher_name"
  exit
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
"./bazel-bin/test/fuzz/$target" \
  -use_value_profile=1 \
  -dict=test/fuzz/ruby.dict \
  -minimize_crash=1 \
  "$crasher" \
  -exact_artifact_path=fuzz_crashers/min/"$crasher_name" \
  "$@" \
  &

child=$!
wait "$child"

if "$cancelled"; then
  echo "cancelled"
else
  mv "$output_file" "$done_file"
fi
