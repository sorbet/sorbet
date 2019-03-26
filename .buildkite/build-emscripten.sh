#!/bin/bash

set -euo pipefail

echo "--- Pre-setup"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == $platform ]]; then
  echo "only mac builds are supported"
  exit 1
elif [[ "mac" == $platform ]]; then
  echo "mac builds are supported"
fi

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/emscripten /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
echo 'common --curses=no --color=yes' >> .bazelrc
echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/emscripten' >> .bazelrc
echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc
echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc

./bazel version

echo "--- compilation"

tools/scripts/update-sorbet.run.sh
