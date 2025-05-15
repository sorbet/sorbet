#!/usr/bin/env bash

set -euo pipefail

echo "--- Pre-setup :bazel:"

case "$(uname -s)" in
  # newer versions of macOS don't allow writing to /usr/local
  Linux*)     usr_local_var="/usr/local/var";;
  Darwin*)    usr_local_var="/opt/homebrew/var";;
  *)          exit 1
esac

if [ "${CLEAN_BUILD:-}" != "" ]; then
  echo "--- cleanup"
  chmod -R 755 "$usr_local_var/bazelcache/"
  rm -rf "$usr_local_var"/bazelcache/*
fi

rm -f bazel-*

function finish {
  ./bazel shutdown
  rm .bazelrc.local
}

mkdir -p \
  "$usr_local_var/bazelcache/output-bases/${JOB_NAME}" \
  "$usr_local_var/bazelcache/build" \
  "$usr_local_var/bazelcache/repos"

cat > .bazelrc.local <<EOF
common --curses=no --color=yes
startup --output_base=$usr_local_var/bazelcache/output-bases/${JOB_NAME}
build  --disk_cache=$usr_local_var/bazelcache/build --repository_cache=$usr_local_var/bazelcache/repos
test   --disk_cache=$usr_local_var/bazelcache/build --repository_cache=$usr_local_var/bazelcache/repos
EOF

./bazel version
PATH=$PATH:$(pwd)
export PATH

echo "+++ ${JOB_NAME}"
