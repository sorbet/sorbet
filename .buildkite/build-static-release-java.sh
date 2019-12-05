#!/bin/bash

set -e 

# Based on the output of build-static-release.sh
# _out_/gems/ should have the Linux & Mac sorbet-static gem
mkdir -p gems/sorbet-static/libexec

for platform in universal-darwin x86_64-linux
do
  gem unpack _out_/gems/sorbet-static-${release_version}-${platform}*.gem

  case $platform in
    x86_64-linux)
        mv sorbet-static-${release_version}-${platform}/libexec/sorbet  gems/sorbet-static/libexec/linux.sorbet
    ;;

    universal-darwin)
        mv sorbet-static-${release_version}-${platform}*/libexec/sorbet gems/sorbet-static/libexec/mac.sorbet
    ;;
  esac

done

pushd gems/sorbet-static

sed -i.bak "s/Gem::Platform::CURRENT/'java'/" sorbet-static.gemspec
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static.gemspec
sed -i.bak "s/'libexec\/sorbet'/'libexec\/mac.sorbet', 'libexec\/linux.sorbet'/" sorbet-static.gemspec

gem build sorbet-static.gemspec

popd

mv gems/sorbet-static/sorbet-static-${release_version}-java.gem _out_/gems
