#!/bin/bash

# This tests that the bundler wrapper produced by the `@gems` package is able to
# install a gem, and provide an environment with that gem in scope.

# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
# shellcheck disable=SC1090
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v2 ---

# This is the name of a ruby bazel package, specified in
# //third_party/externals.bzl
ruby_package=$1

# put bundle in the path
PATH="$(dirname "$(rlocation "{{workspace}}/bundler/bundle")"):$PATH"

# put ruby in the path
RUBY_BIN="$(rlocation "$ruby_package/ruby")"
PATH="$(dirname "$RUBY_BIN"):$PATH"

export PATH

sandbox="$(mktemp -d)"
pushd "$sandbox" > /dev/null

# bundle requires $HOME
HOME="$sandbox"
export HOME

BUNDLE_PATH="$sandbox/bundler"
export BUNDLE_PATH

# ensure that bundle works
bundle --help

cp "$(rlocation "{{workspace}}/test/Gemfile")" .

cp "$(rlocation "{{workspace}}/test/Gemfile.lock")" .

mkdir vendor

VENDOR_CACHE=$(dirname "$(rlocation "{{workspace}}/test/vendor/cache/cantor-1.2.1.gem")")
ln -s "$VENDOR_CACHE" vendor/cache

# setup 'cantor'
bundle install --local --no-prune

# test that the environment is correct
bundle exec ruby -e 'require "cantor"; puts "success"'

popd > /dev/null

rm -rf "$sandbox"
