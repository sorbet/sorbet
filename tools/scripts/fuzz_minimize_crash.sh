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

mkdir -p fuzz_crashers/fixed/min fuzz_crashers/fixed/original

file_arg="$(basename "$crasher")"
crash_full_path="$(realpath "$crasher")"
output_file="fuzz_crashers/min/${file_arg}"
if [ -f "${output_file}.done" ]; then
    echo "already minimized"
    if "bazel-bin/test/fuzz/fuzz_dash_e" "${output_file}.done"; then
      echo "already fixed"
      mv "${output_file}.done" "fuzz_crashers/fixed/min/${file_arg}"
    fi
    exit 0
fi
if [ -f "$output_file" ]; then
    echo "Reusing previous minimized state"
    crash_full_path=$(mktemp)
    cp "$output_file" "$crash_full_path"
fi

if "bazel-bin/test/fuzz/fuzz_dash_e" "${crash_full_path}" ${FUZZ_ARG}; then
  echo "already fixed"
  mv "${crash_full_path}" "fuzz_crashers/fixed/original/${file_arg}"
  exit 0
fi

interrupted=

handler_int()
{
    kill -INT "$child" 2>/dev/null
    interrupted=1
}

handler_term()
{
    kill -TERM "$child" 2>/dev/null
    interrupted=1
}

trap handler_int SIGINT
trap handler_term SIGTERM

mkdir -p "fuzz_crashers/min/"
PATH=$PATH:$(pwd)/bazel-sorbet/external/llvm_toolchain/bin/
export PATH

command -v llvm-symbolizer >/dev/null 2>&1 || { echo 'will need llvm-symbolizer' ; exit 1; }

(
    ASAN_OPTIONS=dedup_token_length=10 ./bazel-bin/test/fuzz/fuzz_dash_e -use_value_profile=1 -dict=test/fuzz/ruby.dict -minimize_crash=1 "$crash_full_path" -exact_artifact_path=fuzz_crashers/min/"$file_arg" ${FUZZ_ARG}
) & # start a subshell that we'll monitor

child=$!
wait "$child"

if [ -z "$interrupted" ]; then
  mv "${output_file}" "${output_file}.done"
fi
