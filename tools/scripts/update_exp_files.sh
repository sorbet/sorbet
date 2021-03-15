#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/../.."

tools/scripts/update_testdata_exp.sh
test/cli/update_cli_exp_files.sh
