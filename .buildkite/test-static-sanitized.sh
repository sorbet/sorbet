#!/bin/bash

set -euo pipefail

if [ "${SIMULATE_FAIL:-}" != "" ]; then
  echo "Simulating build failure. Exiting status 1"
  exit 1
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac


if [[ "linux" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

export JOB_NAME=test-static-sanitized
source .buildkite/tools/setup-bazel.sh

echo will run with $CONFIG_OPTS

err=0

mkdir -p _out_

# NOTE: we skip the compiler tests because llvm doesn't interact well with the sanitizer
./bazel test \
  --experimental_generate_json_trace_profile --profile=_out_/profile.json \
  --test_tag_filters=-compiler \
  --build_tag_filters=-compiler \
  --build_tests_only \
  --test_verbose_timeout_warnings \
  @gems//... \
  //gems/sorbet/test/snapshot \
  //gems/sorbet/test/hidden-method-finder \
  //... $CONFIG_OPTS --test_summary=terse || err=$?

echo "--- uploading test results"

rm -rf _tmp_
mkdir -p _tmp_/log/junit/

# TODO: does this query omit the snapshot tests?
./bazel query 'tests(//...) except (attr("tags", "manual", //...) + attr("tags", "compiler", //...))' | while read -r line; do
    path="${line/://}"
    path="${path#//}"
    cp "bazel-testlogs/$path/test.xml" _tmp_/log/junit/"${path//\//_}-${BUILDKITE_JOB_ID}.xml"
done

annotation_dir="$(mktemp -d "junit-annotate-plugin-annotation-tmp.XXXXXXXXXX")"
annotation_path="${annotation_dir}/annotation.md"

function cleanup {
  rm -rf "${annotation_dir}"
}

trap cleanup EXIT


rbenv install --skip-existing
.buildkite/tools/annotate.rb _tmp_/log/junit > "$annotation_path"

cat "$annotation_path"

if grep -q "<details>" "$annotation_path"; then
  echo "--- :buildkite: Creating annotation"
  # shellcheck disable=SC2002
  cat "$annotation_path" | buildkite-agent annotate --context junit-${platform} --style error --append
fi

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
