#!/bin/bash

# A templated wrapper for the `bundle` command that will work with `bazel run`
# and as an `sh_binary` source.


# was this invoked via bazel run?
RUNFILES="${BASH_SOURCE[0]}.runfiles"
if [ -d "${RUNFILES}" ]; then
  base_dir="${RUNFILES}/{{workspace}}/{{bundler}}"
else
  base_dir="$(dirname "${BASH_SOURCE[0]}")"
fi

RUBY_VERSION=$(ruby -e 'require "rbconfig"; puts RbConfig::CONFIG["ruby_version"]')

BUNDLER_ROOT="${base_dir}/lib/ruby/site_ruby/${RUBY_VERSION}"
export BUNDLER_ROOT

RUBYLIB="${BUNDLER_ROOT}${RUBYLIB:+:}${RUBYLIB:-}"
export RUBYLIB

exec "${base_dir}/{{site_bin}}/bundle" "$@"
