#!/usr/bin/env bash

set -euo pipefail

test_dir="$(dirname "${BASH_SOURCE[0]}")"
root_dir="${test_dir}/../../.."

# shellcheck source=SCRIPTDIR/../../logging.sh
source "${root_dir}/test/logging.sh"

info "Building the exp test outputs"
bazel build //test/cli/compiler:actual_compiler > /dev/null 2>&1

info "Updating exp files"

if [[ -n "${EMIT_SYNCBACK:-}" ]]; then
  echo '### BEGIN SYNCBACK ###'
fi

bazel cquery 'filter("\.out$", deps("//test/cli/compiler:actual_compiler", 1))' 2>/dev/null | \
  cut -d ' ' -f 1 | \
  while read -r target; do
    target="${target#//}"
    target="${target/://}"

    output="${target%actual.out}expected.out"

    echo "${output}"

    cp "bazel-bin/${target}" "${output}"
  done

if [[ -n "${EMIT_SYNCBACK:-}" ]]; then
  echo '### END SYNCBACK ###'
fi

success "Done"
