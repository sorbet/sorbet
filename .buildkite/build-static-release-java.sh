#!/bin/bash

# exit on the first error
# this makes sure we don't create a java gem missing
# either linux or mac
set -o errexit

echo "--- Dowloading artifacts"
rm -rf release
rm -rf _out_
buildkite-agent artifact download "_out_/**/*" .

# Based on the output of build-static-release.sh
# _out_/gems/ should have the Linux & Mac sorbet-static gem
mkdir -p gems/sorbet-static/libexec

git_commit_count=$(git rev-list --count HEAD)
prefix="0.5"
release_version="$prefix.${git_commit_count}"

rbenv install --skip-existing

# we pin to universal-darwin-18 but it shouldn't matter
# the specific version; we just want one mac binary
for platform in universal-darwin-18 x86_64-linux
do
  gem unpack _out_/gems/sorbet-static-"${release_version}"-${platform}*.gem

  case $platform in
    x86_64-linux)
        mv sorbet-static-"${release_version}"-${platform}/libexec/sorbet  gems/sorbet-static/libexec/linux.sorbet
    ;;

    universal-darwin-18)
        mv sorbet-static-"${release_version}"-${platform}*/libexec/sorbet gems/sorbet-static/libexec/mac.sorbet
    ;;
  esac

done

pushd gems/sorbet-static

sed -i.bak "s/Gem::Platform::CURRENT/'java'/" sorbet-static.gemspec
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static.gemspec
sed -i.bak "s/'libexec\/sorbet'/'libexec\/mac.sorbet', 'libexec\/linux.sorbet'/" sorbet-static.gemspec

gem build sorbet-static.gemspec

popd

mv gems/sorbet-static/sorbet-static-"${release_version}"-java.gem _out_/gems
