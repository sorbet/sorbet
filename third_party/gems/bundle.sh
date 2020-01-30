#!/bin/bash

# A templated wrapper for the `bundle` command that will work with `bazel run`
# and as an `sh_binary` source.

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

BUNDLE="$(rlocation "{{workspace}}/bundler/{{site_bin}}/bundle")"

base_dir="$(dirname "$BUNDLE")/.."

RUBY_VERSION=$(ruby -e 'require "rbconfig"; puts RbConfig::CONFIG["ruby_version"]')

BUNDLER_ROOT="${base_dir}/lib/ruby/site_ruby/${RUBY_VERSION}"
export BUNDLER_ROOT

RUBYLIB="${BUNDLER_ROOT}${RUBYLIB:+:}${RUBYLIB:-}"
export RUBYLIB

exec "$BUNDLE" "$@"
