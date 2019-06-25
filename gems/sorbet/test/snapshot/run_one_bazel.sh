#!/bin/bash

# Runs a single srb init test from gems/sorbet/test/snapshot/{partial,total}/*

shopt -s dotglob

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

repo_root="$PWD"

# NOTE: using rlocation here because this script gets run from a genrule
# shellcheck disable=SC1090
source "$(rlocation com_stripe_ruby_typer/gems/sorbet/test/snapshot/logging.sh)"

# ----- Option parsing -----

# these positional arguments are supplied in snapshot.bzl
ruby_package=$1
output_archive="${repo_root}/$2"
test_name=$3

if [[ "${test_name}" =~ partial/* ]]; then
  is_partial=1
else
  is_partial=
fi

info "├─ test_name:  ${test_name}"
info "├─ output_archive:  ${output_archive}"
info "├─ is_partial: ${is_partial:-0}"

# ----- Environment setup -----

# Add ruby to the path
# shellcheck disable=SC1090
PATH="${repo_root}/$(dirname "$(rlocation "${ruby_package}/ruby")"):$PATH"
export PATH

# Add bundler to the path
# shellcheck disable=SC1090
BUNDLER_LOC="${repo_root}/$(dirname "$(rlocation "gems/bundler/bundle")")"
GEMS_LOC="$BUNDLER_LOC/../gems"
PATH="$BUNDLER_LOC:$PATH"
export PATH

test_dir="${repo_root}/gems/sorbet/test/snapshot/${test_name}"

actual="${test_dir}/actual"

srb="${repo_root}/gems/sorbet/bin/srb"

# Use the sorbet executable built by bazel
SRB_SORBET_EXE="$PWD/$(rlocation com_stripe_ruby_typer/main/sorbet)"
export SRB_SORBET_EXE

HOME=$test_dir
export HOME

XDG_CACHE_HOME="${test_dir}/cache"
export XDG_CACHE_HOME

info "├─ ruby: $(command -v ruby)"
info "├─ ruby --version: $(ruby --version)"

info "├─ bundle: $(command -v bundle)"
info "├─ bundle --version: $(bundle --version)"

info "├─ sorbet: $SRB_SORBET_EXE"
info "├─ sorbet --version: $("$SRB_SORBET_EXE" --version)"


# ----- Build the test sandbox -----

cp -r "${test_dir}/src" "$actual"


# ----- Run the test -----

(
  cd "$actual"

  # Setup the vendor/cache directory to include all gems required for any test
  info "├─ Setting up vendor/cache"
  mkdir vendor
  ln -s "$GEMS_LOC" "vendor/cache"

  info "├─ Testing 'bundle exec which ruby'"
  if bundle exec which ruby ; then
    info "├─ success"
  else 
    error "└─ failure"
    exit 1
  fi

  # Configuring output to vendor/bundle
  # Passing --local to never consult rubygems.org
  # Passing --no-prune to not delete unused gems in vendor/cache
  info "├─ Installing dependencies to vendor/bundle"
  bundle config --local path vendor/bundle
  bundle install --local --no-prune

  info "├─ Checking installation"
  bundle check

  # Uses /dev/null for stdin so any binding.pry would exit immediately
  # (otherwise, pry will be waiting for input, but it's impossible to tell
  # because the pry output is hiding in the *.log files)
  #
  # note: redirects stderr before the pipe
  if ! SRB_YES=1 bundle exec "$srb" init < /dev/null 2> "err.log" > "out.log"; then
    error "├─ srb init failed."
    error "├─ stdout (out.log):"
    cat "out.log"
    error "├─ stderr (err.log):"
    cat "err.log"
    error "└─ (end stderr)"
    exit 1
  fi

  # FIXME: Removing hidden-definitions in actual to hide them from diff output.
  rm -rf "sorbet/rbi/hidden-definitions"

  # Fix up the logs to not have sandbox directories present.

  info "├─ Fixing up err.log"
  err_filtered="$(mktemp)"
  sed -e "s,${TMPDIR}[^ ]*/\([^/]*\),<tmp>/\1,g" \
    -e "s,${XDG_CACHE_HOME},<cache>,g" \
    -e "s,${HOME},<home>,g" \
    < "err.log" > "$err_filtered"
  mv "$err_filtered" "err.log"

  info "├─ Fixing up out.log"
  out_filtered="$(mktemp)"
  sed -e 's/with [0-9]* modules and [0-9]* aliases/with X modules and Y aliases/' \
    -e "s,${TMPDIR}[^ ]*/\([^/]*\),<tmp>/\1,g" \
    -e "s,${XDG_CACHE_HOME},<cache>,g" \
    -e "s,${HOME},<home>,g" \
    < "out.log" > "$out_filtered"
  mv "$out_filtered" "out.log"

)

(
  cd "$test_dir"

  info "├─ archiving results"

  # archive the test
  tar -cz actual/{sorbet,err.log,out.log} > "$output_archive"
)

success "└─ done"
