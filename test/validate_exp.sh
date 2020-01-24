#!/bin/bash

set -euo pipefail

# This script is always run from the repo root, so `logging.sh` doesn't exist
# where shellcheck thinks it does.
# shellcheck disable=SC1091
source "test/logging.sh"

# Argument Parsing #############################################################

# Positional arguments
build_archive=${1/--build_archive=/}
shift 1

# sources make up the remaining argumenets
rb=( "$@" )

# Environment Setup ############################################################

root=$PWD

# The directory to unpack the build archive to
target="$(mktemp -d)"

diff_dir="$(mktemp -d)"

# Main #########################################################################

info "--- Unpacking Build ---"
tar -xvf "${build_archive}" -C "${target}"

info "--- Checking Build ---"
pushd "$target" > /dev/null
for ext in "llo"; do
  exp="$root/${rb[0]%.rb}.$ext.exp"
  if [ -f "$exp" ]; then
    actual=(*".$ext")
    if [ ! -f "${actual[0]}" ]; then
      fatal "No LLVMIR found at" "${actual[@]}"
    fi
    if [[ "$OSTYPE" != "darwin"* ]]; then
      diff_out="${diff_dir}/${ext}.diff"
      if diff -u "$exp" <(grep -v '^target triple =' < "${actual[@]}") > "$diff_out"; then
        success "* $(basename "$exp")"
      else
        cat "$diff_out"
        fatal "* $(basename "$exp")"
      fi
    fi
  fi
done
popd > /dev/null

info "Cleaning up temp files"
rm -r "$target" "$diff_dir"

success "Test passed"
