#!/bin/bash

# Runs a single hidden methods test

set -euo pipefail
shopt -s dotglob

# ----- Option parsing -----

# This script is always invoked by bazel at the repository root
repo_root="$PWD"

# these positional arguments are supplied in snapshot.bzl
ruby_package=$1
output_file="${repo_root}/$2"
test_name=$3

# This is the root of the test -- the src and expected directories are
# sub-directories of this one.
test_dir="${repo_root}/gems/sorbet/test/hidden-method-finder/${test_name}"

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

# NOTE: using rlocation here because this script gets run from a genrule
# shellcheck disable=SC1090
source "$(rlocation com_stripe_ruby_typer/gems/sorbet/test/hidden-method-finder/logging.sh)"

# ----- Environment setup and validation -----

HOME=$test_dir
export HOME

XDG_CACHE_HOME="${test_dir}/cache"
export XDG_CACHE_HOME

info "├─ test_name:      ${test_name}"
info "├─ output_file:    ${output_file}"


# Add ruby to the path
RUBY_WRAPPER_LOC="${repo_root}/$(rlocation "${ruby_package}/ruby")"
PATH="$(dirname "$RUBY_WRAPPER_LOC"):$PATH"
export PATH

info "├─ ruby:           $(command -v ruby)"
info "├─ ruby --version: $(ruby --version)"

# Add bundler to the path
BUNDLER_LOC="${repo_root}/$(dirname "$(rlocation gems/bundler/bundle)")"
PATH="$BUNDLER_LOC:$PATH"
export PATH

info "├─ bundle:           $(command -v bundle)"
info "├─ bundle --version: $(bundle --version)"

# Use the sorbet executable built by bazel
SRB_SORBET_EXE="${repo_root}/$(rlocation com_stripe_ruby_typer/main/sorbet)"
export SRB_SORBET_EXE

hidden_method_finder="${repo_root}/gems/sorbet/lib/hidden-definition-finder.rb"

info "├─ sorbet:           $SRB_SORBET_EXE"
info "├─ sorbet --version: $("$SRB_SORBET_EXE" --version)"

# ----- Build the test sandbox -----

# NOTE: this builds a replica of the `src` tree in the `actual` directory, and
# then uses that as a workspace.
actual="${test_dir}/actual"
cp -r "${test_dir}/src" "$actual"

cleanup() {
  rm -rf "$actual"
}

trap cleanup EXIT


# ----- Run the test -----

(
  cd "$actual"

  ruby_loc=$(bundle exec which ruby)
  if [[ "$ruby_loc" == "$RUBY_WRAPPER_LOC" ]] ; then
    info "├─ Bundle was able to find ruby"
  else
    attn "├─ ruby in path:  ${ruby_loc}"
    attn "├─ expected ruby: ${RUBY_WRAPPER_LOC}"
    error "└─ Bundle failed to find ruby"
    exit 1
  fi

  # Configuring output to vendor/bundle
  # Passing --local to never consult rubygems.org
  # Passing --no-prune to not delete unused gems in vendor/cache
  info "├─ Installing dependencies to vendor/bundle"
  bundle install --local --no-prune

  info "├─ Checking installation"
  bundle check

  # Uses /dev/null for stdin so any binding.pry would exit immediately
  # (otherwise, pry will be waiting for input, but it's impossible to tell
  # because the pry output is hiding in the *.log files)
  #
  # note: redirects stderr before the pipe
  if ! bundle exec "$hidden_method_finder" < /dev/null 2> "err.log" > "out.log"; then
    error "├─ hidden method finder failed."
    error "├─ stdout (out.log):"
    cat "out.log"
    error "├─ stderr (err.log):"
    cat "err.log"
    error "└─ (end stderr)"
    exit 1
  fi
)

(
  cd "$test_dir"

  info "├─ exporting results to $output_file"

  # export the results
  mv "${actual}/sorbet/rbi/hidden-definitions/hidden.rbi" "$output_file"
)

success "└─ done"
