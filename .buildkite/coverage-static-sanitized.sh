#!/bin/bash

set -euo pipefail

if [[ -n "${CLEAN_BUILD-}" ]]; then
  echo "--- cleanup"
  rm -rf /usr/local/var/bazelcache/*
fi

echo "--- Pre-setup :bazel:"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
  echo "linux coverage supported"
elif [[ "mac" == "$platform" ]]; then
  echo "mac coverage is not supported"
  exit 1
fi

git checkout .bazelrc

# This clean sidesteps a bug in bazel not re-building correct coverage for cached items
./bazel clean

rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/coverage /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
{
  echo 'common --curses=no --color=yes'
  echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/coverage'
} >> .bazelrc

./bazel version
./bazel clean

echo "+++ tests"

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

rm -rf ./_tmp_/profdata_combined.profdata
xargs .buildkite/combine-coverage.sh < _tmp_/reports

./bazel-sorbet/external/llvm_toolchain/bin/llvm-cov show -instr-profile ./_tmp_/profdata_combined.profdata ./bazel-bin/test/test_corpus_sharded -object ./bazel-bin/main/sorbet > combined.coverage.txt

.buildkite/codecov-bash -f combined.coverage.txt -X search

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
