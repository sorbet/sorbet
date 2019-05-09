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
  command -v autoconf >/dev/null 2>&1 || brew install autoconf
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
  rbenv exec bundle
  rbenv exec bundle exec rake test
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

find /usr/local/var/bazelcache/build/ -type f -size +70M -exec rm {} \;
