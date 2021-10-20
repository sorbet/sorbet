#!/usr/bin/env bash
set -eu -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
RUN_TESTS_JS="$SCRIPT_DIR/../out/scripts/runTests.js"

# Forward all arguments to the script.
exec node "$RUN_TESTS_JS" "$@"

