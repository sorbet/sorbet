#!/bin/bash

set -euo pipefail

if [[ -n "${CLEAN_BUILD-}" ]]; then
  echo "--- cleanup"
  rm -rf /usr/local/var/bazelcache/*
fi

echo "--- Pre-setup"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
  echo "only mac builds are supported"
  exit 1
elif [[ "mac" == "$platform" ]]; then
  echo "mac builds are supported"
  command -v node >/dev/null 2>&1 || brew install node
  command -v realpath >/dev/null 2>&1 || brew install realpath
fi

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/emscripten /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
{
  echo 'common --curses=no --color=yes'
  echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/emscripten'
  echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
  echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
} >> .bazelrc

./bazel version

echo "--- compilation"
PATH=$PATH:$(pwd)
export PATH
tools/scripts/update-sorbet.run.sh

rm -rf _out_
mkdir -p _out_/webasm
cp bazel-bin/emscripten/sorbet-wasm.tar _out_/webasm/sorbet-wasm.tar

find /usr/local/var/bazelcache/build/ -type f -size +17M -exec rm {} \;
