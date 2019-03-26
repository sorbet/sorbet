#!/bin/bash

echo "--- Pre-setup"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == $platform ]]; then
  apt-get install pkg-config zip g++ zlib1g-dev unzip python
  CONFIG_OPTS="--buildfarm-sanitized-linux"
elif [[ "mac" == $platform ]]; then
  CONFIG_OPTS="--buildfarm-sanitized-mac"
fi

echo will run with $CONFIG_OPTS
./bazel version


echo "--- compilation"
./bazel build //... $CONFIG_OPTS

echo "+++ tests"
./bazel build //... $CONFIG_OPTS

