#!/bin/bash

set -euo pipefail
export JOB_NAME=linters

# shellcheck source=SCRIPTDIR/tools/setup-bazel.sh
source .buildkite/tools/setup-bazel.sh

set -x
globalErr=0

echo "~~~ Checking build files"
if ! ./tools/scripts/format_build_files.sh -t &> buildifier; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/format_build_files.sh --style error --append < buildifier
fi

echo "~~~ Checking c++ formatting"
if ! ./tools/scripts/format_cxx.sh -t &> format_cxx; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/format_cxx.sh --style error --append < format_cxx
fi

echo "~~~ Checking that the compilation db builds"
if ! ./tools/scripts/build_compilation_db.sh &> compdb; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/build_compilation_db.sh --style error --append < compdb
fi

echo "~~~ Checking compilation db targets"
if ! ./tools/scripts/generate_compdb_targets.sh -t &> compdb-targets; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/generate_compdb_targets.sh --style error --append < compdb-targets
fi

echo "~~~ Linting uses of \`using namespace std\`"
if ! ./tools/scripts/check_using_namespace_std.sh &> std_check; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/check_using_namespace_std.sh --style error --append < std_check
fi

echo "~~~ Checking ErrorClass error code numbers"
if ! ./tools/scripts/check_error_classes.sh &> error_class_check; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/check_error_classes.sh --style error --append < error_class_check
fi

echo "~~~ Running shellcheck"
if ! ./tools/scripts/lint_sh.sh -t &> lint_sh; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/lint_sh.sh --style error --append < lint_sh
fi

echo "~~~ Checking markdown formatting"
if ! ./tools/scripts/format_website.sh -t &> format_website; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context tools/scripts/format_website.sh --style error --append < format_website
fi

echo "~~~ Checking the vscode extension"
pushd vscode_extension
yarn install
if ! yarn lint --output-file=yarn_lint; then
    globalErr=1
    echo "^^^ +++"
    buildkite-agent annotate --context 'yarn lint' --style error --append <<EOF
There were eslint errors in vscode_extension/. Fix with:

    cd vscode_extension && yarn lint --fix

$(< yarn_lint)
EOF
fi
popd

echo "~~~"
if [ "$globalErr" -ne 0 ]; then
    exit $globalErr
fi
