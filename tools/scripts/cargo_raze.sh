#!/bin/bash
set -euxo pipefail
cd "$(git rev-parse --show-toplevel)"
(
cd third_party/cargo && cargo raze
)
./tools/scripts/format_build_files.sh
(
cd third_party/cargo && ruby add_crates_shas.rb
)
./tools/scripts/format_build_files.sh
