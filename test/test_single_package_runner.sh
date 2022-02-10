#!/bin/bash
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
  # shellcheck disable=SC1090,SC1091
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

# shellcheck source-path=SCRIPTDIR/..
source "test/logging.sh"

# Argument Parsing #############################################################

test_directory=$1

# Environment Setup ############################################################

root="$PWD"

sorbet="$(rlocation com_stripe_ruby_typer/main/sorbet)"
ruby="$(rlocation sorbet_ruby_2_7/toolchain/bin/ruby)"
sorbet_runtime="$(dirname $(rlocation com_stripe_ruby_typer/gems/sorbet-runtime/lib/sorbet-runtime.rb))"
rbi_gen_package_runner="$(rlocation com_stripe_ruby_typer/test/rbi_gen_package_runner.rb)"

# Main #########################################################################

exec "$ruby" -I "${sorbet_runtime}" "${rbi_gen_package_runner}" --sorbet "${sorbet}" --root="$root" --test-directory="${test_directory}"
