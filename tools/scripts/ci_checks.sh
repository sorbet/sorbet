#!/bin/bash
set -eux

./tools/scripts/format_build_files.sh -t
./tools/scripts/format_cxx.sh -t
./tools/scripts/lint_sh.sh -t
./tools/scripts/check_using_namespace_std.sh
./tools/scripts/generate_compdb_targets.sh -t
./tools/scripts/build_compilation_db.sh
