#!/bin/bash

set -euo pipefail

export JOB_NAME=build-static-release
source .buildkite/tools/setup-bazel.sh

kernel_name="$(uname -s | tr 'A-Z' 'a-z')"
processor_name="$(uname -m)"

platform="${kernel_name}-${processor_name}"
case "$platform" in
  linux-x86_64)
    CONFIG_OPTS="--config=release-linux"
    ;;
  linux-aarch64)
    CONFIG_OPTS="--config=release-${platform}"
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

if [ "$kernel_name" != "darwin" ]; then
  cp bazel-bin/main/sorbet sorbet_bin
else
  cp bazel-bin/main/sorbet "sorbet.$platform"

  # TODO(jez) building on arm64 is not tested (no arm64 mac buildkite instances)
  case "$processor_name" in
    x86_64) cross_target=darwin-arm64  ;;
    arm64)  cross_target=darwin-x86_64 ;;
  esac

  ./bazel build //main:sorbet --strip=always "$CONFIG_OPTS" --config="cross-to-$cross_target"
  cp bazel-bin/main/sorbet "sorbet.$cross_target"

  lipo -create -output sorbet_bin sorbet.darwin-*
fi

mkdir gems/sorbet-static/libexec/
cp sorbet_bin gems/sorbet-static/libexec/sorbet

rbenv install --skip-existing

pushd gems/sorbet-static
git_commit_count=$(git rev-list --count HEAD)
release_version="0.5.${git_commit_count}"
sed -i.bak "s/0\\.0\\.0/${release_version}/" sorbet-static.gemspec
if [[ "darwin" == "$kernel_name" ]]; then
    sed -i.bak "s/Gem::Platform::CURRENT/'universal-darwin'/" sorbet-static.gemspec
    gem build sorbet-static.gemspec
    mv sorbet-static.gemspec.bak sorbet-static.gemspec
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
  rbenv exec gem install ../../gems/sorbet-static/sorbet-static-*-"universal-darwin".gem
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
if [[ "$platform" == "linux-x86_64" ]]; then
  mv gems/sorbet/sorbet*.gem _out_/gems/
fi

if [ "$kernel_name" = "darwin" ]; then
  mkdir -p _out_/darwin-universal
  cp sorbet_bin _out_/darwin-universal/

  for plat in {darwin-x86_64,darwin-arm64}; do
    mkdir -p "_out_/$plat"
    cp "sorbet.$plat" "_out_/$plat/"
  done
else
  mkdir -p "_out_/$platform"
  cp sorbet_bin "_out_/$platform/"
fi
