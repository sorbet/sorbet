#!/bin/bash

set -euo pipefail

echo "--- Pre-setup :bazel:"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == $platform ]]; then
  apt-get update -yy
  apt-get install -yy pkg-config zip g++ zlib1g-dev unzip python
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == $platform ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

echo will run with $CONFIG_OPTS

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/test-pr /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
echo 'common --curses=no --color=yes' >> .bazelrc
echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/test-pr' >> .bazelrc
echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc
echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc

./bazel version

echo "--- compilation"
./bazel build //... $CONFIG_OPTS

echo "+++ tests"
./bazel test //... $CONFIG_OPTS

