#!/usr/bin/env bash

set -euo pipefail

echo "--- Pre-setup :bazel:"

if [ "${CLEAN_BUILD:-}" != "" ]; then
  echo "--- cleanup"
  chmod -R 755 /usr/local/var/bazelcache/
  rm -rf /usr/local/var/bazelcache/*
fi

rm -f bazel-*

function finish {
  ./bazel shutdown
  rm .bazelrc.local
}

mkdir -p "/usr/local/var/bazelcache/output-bases/${JOB_NAME}" /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
{
  echo 'common --curses=no --color=yes'
  echo "startup --output_base=/usr/local/var/bazelcache/output-bases/${JOB_NAME}"
  echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
  echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
} > .bazelrc.local

./bazel version
PATH=$PATH:$(pwd)
export PATH

echo "+++ ${JOB_NAME}"
