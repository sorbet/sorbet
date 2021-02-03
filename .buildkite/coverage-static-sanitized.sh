#!/bin/bash

set -exuo pipefail

export JOB_NAME=coverage-static-sanitized
source .buildkite/tools/setup-bazel.sh

# This clean sidesteps a bug in bazel not re-building correct coverage for cached items
./bazel clean

err=0
./bazel coverage //... --config=coverage --config=buildfarm --javabase=@embedded_jdk//:jdk || err=$?  # workaround https://github.com/bazelbuild/bazel/issues/6993

echo "--- uploading coverage results"

rm -rf _tmp_
mkdir _tmp_
touch _tmp_/reports

./bazel query 'tests(//...) except attr("tags", "manual", //...)' | while read -r line; do
    path="${line/://}"
    path="${path#//}"
    echo "bazel-testlogs/$path/coverage.dat" >> _tmp_/reports
done

find ./bazel-app/external/llvm_toolchain_10_0_0/

rm -rf ./_tmp_/profdata_combined.profdata
xargs .buildkite/tools/combine-coverage.sh < _tmp_/reports

./bazel-app/external/llvm_toolchain_10_0_0/bin/llvm-cov show -instr-profile ./_tmp_/profdata_combined.profdata ./bazel-bin/test/test_corpus_sharded -object ./bazel-bin/main/sorbet > combined.coverage.txt

.buildkite/tools/codecov-bash -f combined.coverage.txt -X search

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
