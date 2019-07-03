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

BUNDLER_ROOT="${base_dir}/{{site_ruby}}/lib"
export BUNDLER_ROOT

RUBYLIB="${BUNDLER_ROOT}:${RUBYLIB:-}"
export RUBYLIB

exec "${base_dir}/{{site_bin}}/bundle" "$@"
