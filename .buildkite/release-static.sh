#!/bin/bash

set -exuo pipefail

if [[ -n "${CLEAN_BUILD-}" ]]; then
  echo "--- cleanup"
  rm -rf /usr/local/var/bazelcache/*
fi

echo "--- Pre-setup :bazel:"

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
  apt-get update -yy
  apt-get install -yy pkg-config zip g++ zlib1g-dev unzip python ruby autoconf
  CONFIG_OPTS="--config=release-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=release-mac"
  brew install autoconf
fi

echo will run with $CONFIG_OPTS

git checkout .bazelrc
rm -f bazel-*
mkdir -p /usr/local/var/bazelcache/output-bases/release /usr/local/var/bazelcache/build /usr/local/var/bazelcache/repos
{
  echo 'common --curses=no --color=yes'
  echo 'startup --output_base=/usr/local/var/bazelcache/output-bases/release'
  echo 'build  --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
  echo 'test   --disk_cache=/usr/local/var/bazelcache/build --repository_cache=/usr/local/var/bazelcache/repos'
} >> .bazelrc

./bazel version

echo "--- compilation"
./bazel build //main:sorbet --strip=always $CONFIG_OPTS

mkdir gems/sorbet-static/libexec/
cp bazel-bin/main/sorbet gems/sorbet-static/libexec/

pushd gems/sorbet-static
gem build sorbet-static.gemspec
popd

pushd gems/sorbet
gem build sorbet.gemspec
if [[ "mac" == "$platform" ]]; then
  rbenv exec gem install ../../gems/sorbet-static/sorbet-static-*.gem
  rbenv exec bundle
  rbenv exec bundle exec rake test
fi
popd

rm -rf _out_
mkdir -p _out_/gems

mv gems/sorbet-static/sorbet-static-*.gem _out_/
if [[ "mac" == "$platform" ]]; then
  mv gems/sorbet/sorbet*.gem _out_/
fi
