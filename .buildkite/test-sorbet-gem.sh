#!/bin/bash

# Using set -u doesn't work well with rbenv
set -eo pipefail

echo "--- Pre-setup"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

buildkite-agent artifact download "_out_/$platform/sorbet" .

mkdir -p gems/sorbet-static/libexec
mv "_out_/$platform/sorbet" gems/sorbet-static/libexec
chmod +x gems/sorbet-static/libexec/sorbet

echo "--- setup :ruby:"

pushd gems/sorbet

# rbenv init has uninitialized variables
eval "$(rbenv init -)"

rbenv shell 2.4.3

echo "+++ tests"

# TODO(jez) test/snapshot/driver.sh is not currently capable of testing the actual gem.
# This currently tests that local development isn't broken.
test/snapshot/driver.sh

popd
