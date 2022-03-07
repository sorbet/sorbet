#!/bin/bash

set -euo pipefail
export JOB_NAME=linters

# shellcheck source=SCRIPTDIR/tools/setup-bazel.sh
source .buildkite/tools/setup-bazel.sh

set -x
globalErr=0

if ! ./tools/scripts/format_build_files.sh -t &> buildifier; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/format_build_files.sh --style error --append < buildifier
fi

if ! ./tools/scripts/format_cxx.sh -t &> format_cxx; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/format_cxx.sh --style error --append < format_cxx
fi

if ! ./tools/scripts/build_compilation_db.sh &> compdb; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/build_compilation_db.sh --style error --append < compdb
fi

if ! ./tools/scripts/generate_compdb_targets.sh -t &> compdb-targets; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/generate_compdb_targets.sh --style error --append < compdb-targets
fi

if ! ./tools/scripts/check_using_namespace_std.sh &> std_check; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/check_using_namespace_std.sh --style error --append < std_check
fi

if ! ./tools/scripts/lint_sh.sh -t &> lint_sh; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/lint_sh.sh --style error --append < lint_sh
fi

if ! ./tools/scripts/format_website.sh -t &> format_website; then
    globalErr=$?
    buildkite-agent annotate --context tools/scripts/format_website.sh --style error --append < format_website
fi

pushd vscode_extension
yarn install
if ! yarn lint --output-file=yarn_lint; then
    globalErr=$?
    buildkite-agent annotate --context 'yarn lint' --style error --append <<EOF
There were eslint errors in vscode_extension/. Fix with:

    cd vscode_extension && yarn lint --fix

$(< yarn_lint)
EOF
fi
popd

if [ "$globalErr" -ne 0 ]; then
    exit $globalErr
fi
