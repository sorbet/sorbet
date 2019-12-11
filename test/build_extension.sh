#!/bin/bash

set -euo pipefail

# --- begin runfiles.bash initialization ---
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

# Find logging with rlocation, as this script is run from a genrule
# shellcheck disable=SC1090
source "$(rlocation com_stripe_sorbet_llvm/test/logging.sh)"

# Argument Parsing #############################################################

# Positional arguments
output_archive=$1
shift 1

# Sources make up the remaining input
ruby_source=( "$@" )

# Environment Setup ############################################################

sorbet="$(rlocation com_stripe_sorbet_llvm/main/sorbet)"

# The script is always run from the repo root
root="$PWD"

# Temporary directory for llvm output
target="$(mktemp -d)"

stdout=$(mktemp)
stderr=$(mktemp)

# Main #########################################################################

info "--- Build Config ---"
info "* Archive: ${output_archive}"
info "* Source:  ${ruby_source[*]}"
info "* Sorbet:  ${sorbet}"
info "* Target:  ${target}"
info "* Stdout:  ${stdout}"
info "* Stderr:  ${stderr}"

info "--- Building Extension ---"
if ! $sorbet --silence-dev-message --no-error-count --llvm-ir-folder="$target" \
  --force-compiled "${ruby_source[@]}" 2> "$stderr" > "$stdout" ; then

  info "--- Stdout ---"
  cat "$stdout"

  info "--- Stderr ---"
  cat "$stderr"

  fatal "* Failed to build extension!"
fi

if [ -z "$(ls -A "$target")" ]; then

  info "--- Stdout ---"
  cat "$stdout"

  info "--- Stderr ---"
  cat "$stderr"

  attn "Is ${ruby_source[*]} marked '# typed: true'?"
  fatal "No output produced by sorbet"
fi

info "--- Building Archive ---"
pushd "$target" > /dev/null
tar -czvf "$root/$output_archive" ./*
popd > /dev/null

info "--- Cleaning up ---"
rm -r "$target"

success "* Built ${ruby_source[*]}"

if echo "$ruby_source" | fgrep hello; then
  exit 1
fi


