#!/bin/bash

# Runs a single srb init test from gems/sorbet/test/snapshot/{partial,total}/*

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

# This script is always invoked by bazel at the repository root
repo_root="$PWD"

# Fix up runfiles dir so that it's stable when we change directories
if [ -n "${RUNFILES_DIR:-}" ]; then
  export RUNFILES_DIR="$repo_root/$RUNFILES_DIR"
fi

if [ -n "${RUNFILES_MANIFEST_FILE:-}" ]; then
  export RUNFILES_MANIFEST_FILE="$repo_root/$RUNFILES_MANIFEST_FILE"
fi

# ----- Option parsing -----

# these positional arguments are supplied in snapshot.bzl
ruby_package=$1
output_archive="${repo_root}/$2"
test_name=$3

# This is the root of the test -- the src and expected directories are
# sub-directories of this one.
test_dir="${repo_root}/gems/sorbet/test/snapshot/${test_name}"

# NOTE: using rlocation here because this script gets run from a genrule
# shellcheck disable=SC1090
source "$(rlocation com_stripe_ruby_typer/gems/sorbet/test/snapshot/logging.sh)"

# shellcheck disable=SC1090
source "$(rlocation com_stripe_ruby_typer/gems/sorbet/test/snapshot/hermetic_tar.sh)"


# ----- Environment setup and validation -----

sandbox="$test_dir/$ruby_package"

if [ -d "$sandbox" ]; then
  attn "├─ Cleaning up old test sandbox"
  rm -rf "$sandbox"
fi

mkdir "$sandbox"

HOME="$sandbox"
export HOME

BUNDLE_PATH="$sandbox/bundler"
mkdir "$BUNDLE_PATH"
export BUNDLE_PATH

XDG_CACHE_HOME="$sandbox/cache"
export XDG_CACHE_HOME

if [[ "${test_name}" =~ partial/* ]]; then
  is_partial=1
else
  is_partial=
fi

info "├─ test_name:      ${test_name}"
info "├─ output_archive: ${output_archive}"
info "├─ is_partial:     ${is_partial:-0}"


# Add ruby to the path
RUBY_WRAPPER_LOC="$(rlocation "${ruby_package}/ruby")"
PATH="$(dirname "$RUBY_WRAPPER_LOC"):$PATH"
export PATH

# Disable ruby warnings to get consistent output between different ruby versions
RUBYOPT="-W0"
export RUBYOPT

info "├─ ruby:           $(command -v ruby)"
info "├─ ruby --version: $(ruby --version)"

# Add bundler to the path
BUNDLER_LOC="$(dirname "$(rlocation "${ruby_package}/bundle")")"
PATH="$BUNDLER_LOC:$PATH"
export PATH

info "├─ bundle:           $(command -v bundle)"
info "├─ bundle --version: $(bundle --version)"

# This is a bit fragile, but it's not clear how to find gems/gems otherwise.
GEMS_LOC="$(dirname "$(rlocation gems/gems/cantor-1.2.1.gem)")"

# Use the sorbet executable built by bazel
SRB_SORBET_EXE="$(rlocation com_stripe_ruby_typer/main/sorbet)"
export SRB_SORBET_EXE

srb="${repo_root}/gems/sorbet/bin/srb"

info "├─ sorbet:           $SRB_SORBET_EXE"
info "├─ sorbet --version: $("$SRB_SORBET_EXE" --version)"

SORBET_TYPED_REV="$(rlocation com_stripe_ruby_typer/gems/sorbet/test/snapshot/sorbet-typed.rev)"
SRB_SORBET_TYPED_REVISION="$(<"$SORBET_TYPED_REV")"
export SRB_SORBET_TYPED_REVISION

if [ "$SRB_SORBET_TYPED_REVISION" = "" ]; then
  error "└─ empty sorbet-typed revision from: ${SORBET_TYPED_REV}"
  exit 1
else
  info "├─ sorbet-typed:     $SRB_SORBET_TYPED_REVISION"
fi


# ----- Build the test sandbox -----

# NOTE: this builds a replica of the `src` tree in the `actual` directory, and
# then uses that as a workspace.
actual="$sandbox/actual"
cp -r "${test_dir}/src/" "$actual"

if [ -d "${test_dir}/gems" ]; then
  cp -r "${test_dir}/gems" "$sandbox"
fi

# ----- Run the test -----

(
  cd "$actual"

  # Setup the vendor/cache directory to include all gems required for any test
  info "├─ Setting up vendor/cache"

  mkdir vendor
  ln -sf "$GEMS_LOC" "vendor/cache"

  ruby_loc=$(bundle exec which ruby | sed -e 's,toolchain/bin/,,')
  if [[ "$ruby_loc" == "$RUBY_WRAPPER_LOC" ]] ; then
    info "├─ Bundle was able to find ruby"
  else
    attn "├─ ruby in path:  ${ruby_loc}"
    attn "├─ expected ruby: ${RUBY_WRAPPER_LOC}"
    error "└─ Bundle failed to find ruby"
    exit 1
  fi

  # Configuring output to vendor/bundle
  # Setting no-prune to not delete unused gems in vendor/cache
  # Passing --local to never consult rubygems.org
  info "├─ Installing dependencies to BUNDLE_PATH"
  bundle config set no_prune true
  bundle install --verbose --local

  info "├─ Checking srb commands"
  bundle check

  # By default we only run `srb init` for each test but they can contain a
  # `test.sh` file with the list of commands to run.
  test_cmd=("bundle" "exec" "$srb" "init")
  test_script="test.sh"
  if [ -f "$test_script" ]; then
    test_cmd=("./$test_script" "$srb")
    chmod +x "$test_script"
  fi

  # Uses /dev/null for stdin so any binding.pry would exit immediately
  # (otherwise, pry will be waiting for input, but it's impossible to tell
  # because the pry output is hiding in the *.log files)
  #
  # note: redirects stderr before the pipe
  info "├─ Running srb"
  # shellcheck disable=SC2048
  if ! SRB_YES=1 ${test_cmd[*]} < /dev/null 2> "err.log" > "out.log"; then
    error "├─ srb failed."
    error "├─ stdout (out.log):"
    cat "out.log"
    error "├─ stderr (err.log):"
    cat "err.log"
    error "└─ (end stderr)"
    exit 1
  fi

  # FIXME: Removing hidden-definitions in actual to hide them from diff output.
  rm -rf "sorbet/rbi/hidden-definitions"

  # Remove empty folders inside sorbet, because they can't be checked into git,
  # so they'll always show up as present in actual but absent in expected.
  find ./sorbet -empty -type d -delete

  # Fix up the logs to not have sandbox directories present.

  info "├─ Fixing up err.log"
  sed -i.bak \
    -e "s,${TMPDIR}[^ ]*/\([^/]*\),<tmp>/\1,g" \
    -e "s,${XDG_CACHE_HOME},<cache>,g" \
    -e "s,${HOME},<home>,g" \
    "err.log"

  info "├─ Fixing up out.log"
  sed -i.bak \
    -e 's/with [0-9]* modules and [0-9]* aliases/with X modules and Y aliases/' \
    -e "s,${TMPDIR}[^ ]*/\([^/]*\),<tmp>/\1,g" \
    -e "s,${XDG_CACHE_HOME},<cache>,g" \
    -e "s,${HOME},<home>,g" \
    "out.log"

  info "├─ archiving results"

  # archive the test
  output="$(mktemp -d)"
  cp -r sorbet err.log out.log "$output"
  hermetic_tar "$output" "$output_archive"
  rm -rf "$output"
)

# cleanup
rm -rf "$sandbox"

success "└─ done"
