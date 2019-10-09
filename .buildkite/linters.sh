#!/bin/bash

set -euo pipefail
export JOB_NAME=linters
source .buildkite/tools/setup-bazel.sh

wget -O - https://github.com/buildkite/terminal-to-html/releases/download/v3.2.0/terminal-to-html-3.2.0-linux-amd64.gz | gunzip -c > ./terminal-to-html
chmod u+x ./terminal-to-html

set -x
err=0
globalErr=0

./tools/scripts/format_build_files.sh -t &> buildifier || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < buildifier; echo '</code></pre>') > buildifier_wrapped
    buildkite-agent annotate --context tools/scripts/format_build_files.sh --style error --append < buildifier_wrapped
    globalErr=$err
fi

err=0
./tools/scripts/format_cxx.sh -t &> format_cxx || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < format_cxx; echo '</code></pre>') > format_cxx_wrapped
    buildkite-agent annotate --context tools/scripts/format_cxx.sh --style error --append < format_cxx_wrapped
    globalErr=$err
fi

err=0
./tools/scripts/build_compilation_db.sh &> compdb || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < compdb; echo '</code></pre>') > compdb_wrapped
    buildkite-agent annotate --context tools/scripts/build_compilation_db.sh --style error --append < compdb_wrapped
    globalErr=$err
fi

err=0
./tools/scripts/generate_compdb_targets.sh -t &> compdb-targets || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < compdb-targets; echo '</code></pre>') > compdb-targets_wrapped
    buildkite-agent annotate --context tools/scripts/generate_compdb_targets.sh --style error --append < compdb-targets_wrapped
    globalErr=$err
fi

err=0
./tools/scripts/check_using_namespace_std.sh &> std_check || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < std_check; echo '</code></pre>') > std_check_wrapped
    buildkite-agent annotate --context tools/scripts/check_using_namespace_std.sh --style error --append < std_check_wrapped
    globalErr=$err
fi

err=0
./tools/scripts/lint_sh.sh -t &> lint_sh || err=$?
if [ "$err" -ne 0 ]; then
    (echo '<pre class="term"><code>'; ./terminal-to-html < lint_sh; echo '</code></pre>') > lint_sh_wrapped
    buildkite-agent annotate --context tools/scripts/lint_sh.sh --style error --append < lint_sh_wrapped
    globalErr=$err
fi

if [ "$globalErr" -ne 0 ]; then
    exit $globalErr
fi
