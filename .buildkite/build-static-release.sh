#!/bin/bash

set -euo pipefail

export JOB_NAME=build-static-release
source .buildkite/tools/setup-bazel.sh

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
  CONFIG_OPTS="--config=release-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=release-mac"
  command -v autoconf >/dev/null 2>&1 || brew install autoconf
fi

echo will run with $CONFIG_OPTS

./bazel build //main:sorbet --strip=always $CONFIG_OPTS

mkdir gems/sorbet-static/libexec/
cp bazel-bin/main/sorbet gems/sorbet-static/libexec/

pushd gems/sorbet-static
git_commit_count=$(git rev-list --count HEAD)
release_version="0.4.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static.gemspec
if [[ "mac" == "$platform" ]]; then
    # Our binary should work on almost all OSes. The oldest v8 publishes is -14
    # so I'm going with that for now.
    for i in {14..18}; do
        sed -i.bak "s/Gem::Platform::CURRENT/'universal-darwin-$i'/" sorbet-static.gemspec
        gem build sorbet-static.gemspec
        mv sorbet-static.gemspec.bak sorbet-static.gemspec
    done
else
    gem build sorbet-static.gemspec
fi
popd

pushd gems/sorbet
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet.gemspec
gem build sorbet.gemspec
if [[ "mac" == "$platform" ]]; then
  rbenv exec gem install ../../gems/sorbet-static/sorbet-static-*-universal-darwin-18.gem
  rbenv exec gem install sorbet-*.gem

  mkdir -p srb-init-smoke-test
  (
    cd srb-init-smoke-test
    SRB_YES=1 rbenv exec srb init
    rbenv exec srb tc -e 'puts 1'

    rm -rf sorbet/

    touch Gemfile
    SRB_YES=1 rbenv exec srb init
    rbenv exec srb tc -e 'puts 1'
  )
  rm -rf srb-init-smoke-test

  rbenv exec bundle
  rbenv exec bundle exec rake test

  rbenv exec gem uninstall --all --executables --ignore-dependencies sorbet sorbet-static
fi
popd

rm -rf _out_
mkdir -p _out_/gems

mv gems/sorbet-static/sorbet-static-*.gem _out_/gems/
if [[ "mac" == "$platform" ]]; then
  mv gems/sorbet/sorbet*.gem _out_/gems/
fi

mkdir -p _out_/$platform
cp bazel-bin/main/sorbet _out_/$platform/
