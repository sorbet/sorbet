#!/bin/bash

set -euo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 1 ]; then
cat <<EOF
usage:
  $0 <target> [<options>]

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

echo "building $target"
./bazel build "//test/fuzz:$target" --config=fuzz -c opt

# we want the bazel build command to run before this check so that bazel can download itself.
export PATH="$PATH:$PWD/bazel-sorbet/external/llvm_toolchain_10_0_0/bin"
if ! command -v llvm-symbolizer >/dev/null; then
  echo "fatal: command not found: llvm-symbolizer"
  exit 1
fi

echo "setting up fuzz_corpus"
mkdir -p fuzz_corpus
find test/testdata -iname "*.rb" | grep -v disable | xargs -n 1 -I % cp % fuzz_corpus
mkdir -p fuzz_crashers/original

# use top 10 frames to tell different errors apart
export ASAN_OPTIONS="dedup_token_length=10"

echo "running"
# use nice to run with decreased priority.
nice "./bazel-bin/test/fuzz/$target" \
  -only_ascii=1 \
  -dict=test/fuzz/ruby.dict \
  -artifact_prefix=fuzz_crashers/original/ \
  "$@" \
  fuzz_corpus
