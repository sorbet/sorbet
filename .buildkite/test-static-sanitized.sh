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
  apt-get update -yy
  apt-get install -yy pkg-config zip g++ zlib1g-dev unzip python ruby
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

echo will run with $CONFIG_OPTS

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/test-pr /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
{
  echo 'common --curses=no --color=yes'
  echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/test-pr'
  echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
  echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
} >> .bazelrc

./bazel version

echo "+++ tests"

err=0
./bazel test //... $CONFIG_OPTS || err=$?

echo "--- uploading test results"

rm -rf _tmp_
mkdir -p _tmp_/log/junit/

./bazel query 'tests(//...) except attr("tags", "manual", //...)' | while read -r line; do
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


.buildkite/annotate.rb _tmp_/log/junit > "$annotation_path"

cat "$annotation_path"

if grep -q "<details>" "$annotation_path"; then
  echo "--- :buildkite: Creating annotation"
  # shellcheck disable=SC2002
  cat "$annotation_path" | buildkite-agent annotate --context junit-${platform} --style error --append
fi

find /usr/local/var/bazelcache/build/ -type f -amin +1440 -exec rm {} \;

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
