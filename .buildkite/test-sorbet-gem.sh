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

git_commit_count=$(git rev-list --count HEAD)
release_version="0.4.${git_commit_count}"

buildkite-agent artifact download "_out_/$platform/sorbet" .
buildkite-agent artifact download "_out_/gems/*.gem" .

SRB_SORBET_EXE="$(realpath "_out_/$platform/sorbet")"
export SRB_SORBET_EXE
chmod +x "$SRB_SORBET_EXE"


echo "--- setup :ruby:"

# rbenv init has uninitialized variables
eval "$(rbenv init -)"
rbenv shell 2.4.3


echo "--- local dev tests"

gems/sorbet/test/snapshot/driver.sh --verbose


echo "--- built gem tests"

unset SRB_SORBET_EXE

if [[ "mac" == "$platform" ]]; then
  gem install "_out_/gems/sorbet-static-$release_version-universal-darwin-18.gem"
else
  gem install "_out_/gems/sorbet-static-$release_version-x86_64-linux.gem"
fi
gem install "_out_/gems/sorbet-$release_version.gem"

gems/sorbet/test/snapshot/driver.sh --verbose --installed

gem uninstall --all --executables --ignore-dependencies sorbet sorbet-static
