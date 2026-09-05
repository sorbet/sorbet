#!/bin/bash

set -euo pipefail

symbolizer="${TEST_SRCDIR}/llvm_toolchain_15_0_7/bin/llvm-symbolizer"
if [[ ! -x "${symbolizer}" ]]; then
    echo "Could not find llvm-symbolizer at ${symbolizer}" >&2
    exit 1
fi

export ASAN_SYMBOLIZER_PATH="${symbolizer}"
exec "$@"
