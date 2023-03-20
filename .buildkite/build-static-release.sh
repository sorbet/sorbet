#!/bin/bash

set -euo pipefail

export JOB_NAME=build-static-release
source .buildkite/tools/setup-bazel.sh

kernel_name="$(uname -s | tr 'A-Z' 'a-z')"
processor_name="$(uname -m)"

platform="${kernel_name}-${processor_name}"
case "$platform" in
  linux-x86_64|linux-aarch64)
    CONFIG_OPTS="--config=release-linux"
    ;;
  darwin-x86_64|darwin-arm64)
    CONFIG_OPTS="--config=release-mac"
    command -v autoconf >/dev/null 2>&1 || brew install autoconf
    ;;
  *)
    echo >&2 "Building on $platform is not implemented"
    exit 1
    ;;
esac

echo will run with $CONFIG_OPTS

./bazel build //main:sorbet --strip=always $CONFIG_OPTS

mkdir gems/sorbet-static/libexec/
cp bazel-bin/main/sorbet gems/sorbet-static/libexec/

rbenv install --skip-existing

pushd gems/sorbet-static
git_commit_count=$(git rev-list --count HEAD)
release_version="0.5.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static.gemspec
if [[ "darwin" == "$kernel_name" ]]; then
    # Our binary should work on almost all OSes. The oldest v8 publishes is -14
    # so I'm going with that for now.
    for i in {14..22}; do
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

# This part tests the actual compiled gem (outside of snapshot harness).
# We can't test this inside of the test harness, because we'd want to run it
# via `bundle exec srb init`, which would require that each Gemfile in our test
# suite list `sorbet` as a dependency. This interacts poorly with local
# development, because then to even run the tests locally you'd have to build
# the gem (which involves building sorbet-static and sorbet).
#
# Thus, we only have these two smoke tests of `srb init` via the whole gem.
# This is to catch environment-specific things, like when making changes to the
# way that the `sorbet-static` libexec/ folder is discovered. Most tests should
# be possible to be written as normal snapshot tests.

# TODO(jez) It's not clear to me why we need these here for the tests to run.
# We should investigate how to not need these dependencies, because `srb init`
# doesn't depend on them except in development.

rbenv exec gem uninstall --all --executables --ignore-dependencies minitest mocha

rbenv exec gem uninstall --all --executables --ignore-dependencies sorbet sorbet-static
trap 'rbenv exec gem uninstall --all --executables --ignore-dependencies sorbet sorbet-static' EXIT

if [[ "darwin" == "$kernel_name" ]]; then
  gem_platform="$(ruby -e "(platform = Gem::Platform.local).cpu = 'universal'; puts(platform.to_s)")"
  rbenv exec gem install ../../gems/sorbet-static/sorbet-static-*-"$gem_platform".gem
else
  rbenv exec gem install ../../gems/sorbet-static/sorbet-static-*-"$processor_name"-linux.gem
fi
rbenv exec gem install sorbet-*.gem

smoke_test_dir="$(mktemp -d)"
# Explicitly want this string to expand eagerly.
# shellcheck disable=SC2064
trap "rm -rf '$smoke_test_dir'" EXIT

cp ../../.ruby-version "$smoke_test_dir"

(
  # Make sure we're in a totally separate tree, otherwise a Gemfile from a
  # parent folder will affect this test.
  cd "$smoke_test_dir"
  touch Gemfile
  SRB_YES=1 rbenv exec srb init
  rbenv exec srb tc -e 'puts 1'
)
rm -rf srb-init-smoke-test

rbenv exec bundle
rbenv exec bundle exec rake test

popd

rm -rf _out_
mkdir -p _out_/gems

mv gems/sorbet-static/sorbet-static-*.gem _out_/gems/
if [[ "$kernel_name" == "linux" ]]; then
  mv gems/sorbet/sorbet*.gem _out_/gems/
fi

mkdir -p _out_/$platform
cp bazel-bin/main/sorbet _out_/$platform/
