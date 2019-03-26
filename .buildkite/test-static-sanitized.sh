#!/bin/bash

echo "--- Pre-setup"
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     CONFIG_OPTS="--config=buildfarm-sanitized-linux";;
    Darwin*)    CONFIG_OPTS="--config=buildfarm-sanitized-mac";;
    *)          exit 1
esac
./bazel version

echo "--- compilation"
./bazel build //... $CONFIG_OPTS

echo "+++ tests"
./bazel build //... $CONFIG_OPTS

