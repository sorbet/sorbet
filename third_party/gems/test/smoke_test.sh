#!/bin/bash

# This tests that the bundler wrapper produced by the `@gems` package is able to
# install a gem, and provide an environment with that gem in scope.

# This is the name of a ruby bazel package, specified in
# //third_party/externals.bzl
ruby_package=$1

# --- begin runfiles.bash initialization ---
# Copy-pasted from Bazel's Bash runfiles library https://github.com/bazelbuild/bazel/blob/defd737761be2b154908646121de47c30434ed51/tools/bash/runfiles/runfiles.bash
set -euo pipefail
if [[ ! -d "${RUNFILES_DIR:-/dev/null}" && ! -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
  if [[ -f "$0.runfiles_manifest" ]]; then
    export RUNFILES_MANIFEST_FILE="$0.runfiles_manifest"
  elif [[ -f "$0.runfiles/MANIFEST" ]]; then
    export RUNFILES_MANIFEST_FILE="$0.runfiles/MANIFEST"
  elif [[ -f "$0.runfiles/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
    export RUNFILES_DIR="$0.runfiles"
  fi
fi
if [[ -f "${RUNFILES_DIR:-/dev/null}/bazel_tools/tools/bash/runfiles/runfiles.bash" ]]; then
  # shellcheck disable=SC1090
  source "${RUNFILES_DIR}/bazel_tools/tools/bash/runfiles/runfiles.bash"
elif [[ -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
  # shellcheck disable=SC1090
  source "$(grep -m1 "^bazel_tools/tools/bash/runfiles/runfiles.bash " \
            "$RUNFILES_MANIFEST_FILE" | cut -d ' ' -f 2-)"
else
  echo >&2 "ERROR: cannot find @bazel_tools//tools/bash/runfiles:runfiles.bash"
  exit 1
fi
# --- end runfiles.bash initialization ---

# put bundle in the path
PATH="$(dirname "$(rlocation "{{workspace}}/bundler/bundle")"):$PATH"

# put ruby in the path
RUBY_BIN="$(rlocation "$ruby_package/ruby")"
PATH="$(dirname "$RUBY_BIN"):$PATH"

export PATH

# bundle requires $HOME
HOME=$PWD
export HOME

# ensure that bundle works
bundle --help

cp "$(rlocation "{{workspace}}/test/Gemfile")" .

cp "$(rlocation "{{workspace}}/test/Gemfile.lock")" .

mkdir vendor

VENDOR_CACHE=$(dirname "$(rlocation "{{workspace}}/test/vendor/cache/cantor-1.2.1.gem")")
ln -s "$VENDOR_CACHE" vendor/cache

# setup 'cantor'
bundle install --deployment --local

# test that the environment is correct
bundle exec ruby -e 'require "cantor"; puts "success"'
