#!/bin/bash

set -exuo pipefail
export JOB_NAME=linters
source .buildkite/tools/setup-bazel.sh

err=0
globalErr=0

./tools/scripts/format_build_files.sh -t &> buildifier || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/format_build_files.sh --style error --append < buildifier
    globalErr=$err
fi

err=0
./tools/scripts/format_cxx.sh -t &> format_cxx || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/format_cxx.sh --style error --append < format_cxx
    globalErr=$err
fi

err=0
./tools/scripts/build_compilation_db.sh &> compdb || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/build_compilation_db.sh --style error --append < compdb
    globalErr=$err
fi

err=0
./tools/scripts/generate_compdb_targets.sh -t &> compdb-targets || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/generate_compdb_targets.sh --style error --append < compdb-targets
    globalErr=$err
fi

err=0
./tools/scripts/check_using_namespace_std.sh &> std_check || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/check_using_namespace_std.sh --style error --append < std_check
    globalErr=$err
fi

err=0
./tools/scripts/lint_sh.sh -t &> lint_sh || err=$?
if [ "$err" -ne 0 ]; then
    buildkite-agent annotate --context tools/scripts/lint_sh.sh --style error --append < lint_sh
    globalErr=$err
fi

if [ "$globalErr" -ne 0 ]; then
    exit $globalErr
fi
