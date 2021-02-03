#!/bin/bash

set -euo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 2 ]; then
cat <<EOF
usage:
  $0 <target> <crasher> [<options>]

example target:
  fuzz_dash_e
  fuzz_doc_symbols
  fuzz_hover

example crasher:
  fuzz_crashers/original/crash-[...]

example options:
  --stress-incremental-resolver
EOF
exit 1
fi

target="$1"
shift
# this realpath is needed when running this script under parallel.
crasher="$(realpath "$1")"
shift

mkdir -p fuzz_crashers/fixed/min fuzz_crashers/fixed/original fuzz_crashers/min

crasher_name="$(basename "$crasher")"
output_file="fuzz_crashers/min/$crasher_name"
done_file="$output_file.done"

if [ -f "$done_file" ]; then
  echo "$crasher: already minimized"
  if "./bazel-bin/test/fuzz/$target" "$done_file"; then
    echo "$crasher: already fixed"
    mv "$done_file" "fuzz_crashers/fixed/min/$crasher_name"
  fi
  exit
fi

if [ -f "$output_file" ]; then
  echo "$crasher: reusing previous minimized state"
  crasher="$(mktemp)"
  cp "$output_file" "$crasher"
fi

if "./bazel-bin/test/fuzz/$target" "$crasher" "$@"; then
  echo "$crasher: already fixed"
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

export PATH="$PATH:$PWD/bazel-sorbet/external/llvm_toolchain_10_0_0/bin"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

# use top 10 frames to tell different errors apart
export ASAN_OPTIONS="dedup_token_length=10"

# start a backgrounded command that we'll monitor
echo "$crasher: running"
# this should not use nice, even though in fuzz.sh we use nice
"./bazel-bin/test/fuzz/$target" \
  -dict=test/fuzz/ruby.dict \
  -minimize_crash=1 \
  -exact_artifact_path="$output_file" \
  "$@" \
  "$crasher" \
  &

child="$!"
wait "$child"

if "$cancelled"; then
  echo "$crasher: cancelled"
else
  mv "$output_file" "$done_file"
  echo "$crasher: done"
fi
