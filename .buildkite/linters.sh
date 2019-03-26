#!/bin/bash

set -euo pipefail

echo "--- Pre-setup :bazel:"

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/linters /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
echo 'common --curses=no --color=yes' >> .bazelrc
echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/linters' >> .bazelrc
echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc
echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos' >> .bazelrc

./bazel version
export PATH=$PATH:$(pwd)

echo "+++ linters"
./tools/scripts/ci_checks.sh
