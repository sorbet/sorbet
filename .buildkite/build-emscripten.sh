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
fi

echo will run with $CONFIG_OPTS
./bazel version


echo "--- compilation"

tools/scripts/update-sorbet.run.sh
