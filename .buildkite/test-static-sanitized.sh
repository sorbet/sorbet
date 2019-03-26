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
./bazel version


echo "--- compilation"
./bazel build //... $CONFIG_OPTS

echo "+++ tests"
./bazel test //... $CONFIG_OPTS

