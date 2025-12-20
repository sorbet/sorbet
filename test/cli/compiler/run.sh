#!/usr/bin/env bash

set -euo pipefail

# --- begin runfiles.bash initialization --- {{{
# Copy-pasted from Bazel's Bash runfiles library https://github.com/bazelbuild/bazel/blob/defd737761be2b154908646121de47c30434ed51/tools/bash/runfiles/runfiles.bash
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
  # shellcheck disable=SC1091
  source "${RUNFILES_DIR}/bazel_tools/tools/bash/runfiles/runfiles.bash"
elif [[ -f "${RUNFILES_MANIFEST_FILE:-/dev/null}" ]]; then
  # shellcheck disable=SC1090
  source "$(grep -m1 "^bazel_tools/tools/bash/runfiles/runfiles.bash " \
            "$RUNFILES_MANIFEST_FILE" | cut -d ' ' -f 2-)"
else
  echo >&2 "ERROR: cannot find @bazel_tools//tools/bash/runfiles:runfiles.bash"
  exit 1
fi
# --- end runfiles.bash initialization --- }}}

root="$PWD"

test_path=$1
test_output="${root}/$2"

# Put sorbet into the path so that it's accessible from the cli test
sorbet="$(dirname "$(rlocation com_stripe_ruby_typer/compiler/sorbet)")"
if ! [[ "$sorbet" = /* ]]; then
  sorbet="${root}/${sorbet}"
fi

# TODO(aprocter): It feels like we should be able to say
# $(rlocation sorbet_ruby_2_7_for_compiler/ruby) here, but when we do that it triggers some
# weird runfiles-related behavior in the wrapper script that rlocation winds up
# pointing us to (the wrapper script exits with "ERROR: cannot find
# @bazel_tools//tools/bash/runfiles:runfiles.bash"). As a workaround, we go
# directly to the binary (sorbet_ruby_2_7_for_compiler/toolchain/bin/ruby) instead of the
# shell wrapper (sorbet_ruby_2_7_for_compiler/ruby).
sorbet_ruby="$(dirname "$(rlocation sorbet_ruby_2_7_for_compiler/toolchain/bin/ruby)")"
if ! [[ "$sorbet_ruby" = /* ]]; then
  sorbet_ruby="${root}/${sorbet_ruby}"
fi

export PATH="${sorbet}:${sorbet_ruby}:$PATH"

# Run the test
cd "${test_path}"
if ! ./test.sh > "${test_output}" 2>&1; then
  cat "${test_output}"
  exit 1
fi
