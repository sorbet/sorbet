#!/bin/bash

set -exuo pipefail

if (( $# != 1 )); then
    echo "Illegal number of parameters. Need a single file to minimize"
fi

FUZZ_ARG="--stress-incremental-resolver" # to run incremental resolver

dir="$( dirname "${BASH_SOURCE[0]}" )"

mkdir -p "${dir}/../../fuzz_crashers/fixed/min/" "${dir}/../../fuzz_crashers/fixed/original/"

file_arg="$(basename "$1")"
crash_full_path="$(realpath "$1")"
output_file="${dir}/../../fuzz_crashers/min/${file_arg}"
if [ -f "${output_file}.done" ]; then
    echo "already minimized"
    if "${dir}/../../bazel-bin/test/fuzz/fuzz_dash_e" "${output_file}.done"; then
      echo "already fixed"
      mv "${output_file}.done" "${dir}/../../fuzz_crashers/fixed/min/${file_arg}"
    fi
    exit 0
fi
if [ -f "$output_file" ]; then
    echo "Reusing previous minimized state"
    crash_full_path=$(mktemp)
    cp "$output_file" "$crash_full_path"
fi

if "${dir}/../../bazel-bin/test/fuzz/fuzz_dash_e" "${crash_full_path}" ${FUZZ_ARG}; then
  echo "already fixed"
  mv "${crash_full_path}" "${dir}/../../fuzz_crashers/fixed/original/${file_arg}"
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

mkdir -p "${dir}/../../fuzz_crashers/min/" 

(
    cd "$dir"/../..
    ASAN_OPTIONS=dedup_token_length=10 ./bazel-bin/test/fuzz/fuzz_dash_e -use_value_profile=1 -dict=test/fuzz/ruby.dict -minimize_crash=1 "$crash_full_path" -exact_artifact_path=fuzz_crashers/min/"$file_arg" ${FUZZ_ARG}
) & # start a subshell that we'll monitor
				  
child=$! 
wait "$child"

if [ -z "$interrupted" ]; then
  mv "${output_file}" "${output_file}.done" 
fi
