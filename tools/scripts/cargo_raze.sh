#!/bin/bash
set -euxo pipefail
cd "$(git rev-parse --show-toplevel)"
(
cd third_party/cargo && cargo raze
)
./tools/scripts/format_build_files.sh
