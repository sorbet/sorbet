#!/bin/bash

# Runs a single hidden methods test

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

# Explicitly setting this after runfiles initialization
set -euo pipefail
shopt -s dotglob

# ----- Option parsing -----

# This script is always invoked by bazel at the repository root
repo_root="$PWD"

# Fix up runfiles dir so that it's stable when we change directories
if [ -n "${RUNFILES_DIR:-}" ]; then
  export RUNFILES_DIR="$repo_root/$RUNFILES_DIR"
fi

if [ -n "${RUNFILES_MANIFEST_FILE:-}" ]; then
  export RUNFILES_MANIFEST_FILE="$repo_root/$RUNFILES_MANIFEST_FILE"
fi

# these positional arguments are supplied in snapshot.bzl
ruby_package=$1
output_file="${repo_root}/$2"
test_name=$3

# This is the root of the test -- the src and expected directories are
# sub-directories of this one.
test_dir="${repo_root}/gems/sorbet/test/hidden-method-finder/${test_name}"

# NOTE: using rlocation here because this script gets run from a genrule
# shellcheck source=SCRIPTDIR/logging.sh
source "$(rlocation com_stripe_ruby_typer/gems/sorbet/test/hidden-method-finder/logging.sh)"

# ----- Environment setup and validation -----

HOME=$test_dir
export HOME

info "├─ test_name:      ${test_name}"
info "├─ output_file:    ${output_file}"


# Add ruby to the path
RUBY_WRAPPER_LOC="$(rlocation "${ruby_package}/ruby")"
PATH="$(dirname "$RUBY_WRAPPER_LOC"):$PATH"
export PATH

info "├─ ruby:           $(command -v ruby)"
info "├─ ruby --version: $(ruby --version)"

# Use the sorbet executable built by bazel
SRB_SORBET_EXE="$(rlocation com_stripe_ruby_typer/main/sorbet)"
export SRB_SORBET_EXE

hidden_method_finder="${repo_root}/gems/sorbet/lib/hidden-definition-finder.rb"

info "├─ sorbet:           $SRB_SORBET_EXE"
info "├─ sorbet --version: $("$SRB_SORBET_EXE" --version)"

# ----- Build the test sandbox -----

# NOTE: this builds a replica of the `src` tree in the `actual` directory, and
# then uses that as a workspace.
actual="${test_dir}/${ruby_package}"
cp -r "${test_dir}/src" "$actual"
cp "${test_dir}/../shims.rb.source" "${actual}/shims.rb"

XDG_CACHE_HOME="${actual}/cache"
export XDG_CACHE_HOME


# ----- Run the test -----

(
  cd "$actual"

  # Uses /dev/null for stdin so any binding.pry would exit immediately
  # (otherwise, pry will be waiting for input, but it's impossible to tell
  # because the pry output is hiding in the *.log files)
  #
  # note: redirects stderr before the pipe
  if ! "$RUBY_WRAPPER_LOC" "$hidden_method_finder" < /dev/null 2> "err.log" > "out.log"; then
    error "├─ hidden method finder failed."
    error "├─ stdout (out.log):"
    cat "out.log"
    error "├─ stderr (err.log):"
    cat "err.log"
    error "└─ (end stderr)"
    exit 1
  fi

  info "├─ exporting results to $output_file"

  # export the results
  mv "sorbet/rbi/hidden-definitions/hidden.rbi" "$output_file"
)

rm -rf "$actual"

success "└─ done"
