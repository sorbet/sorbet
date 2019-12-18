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
    if [[ "$OSTYPE" == "darwin"* ]]; then
      if diff -u <(grep -v '^target triple =' < "${actual[@]}") "$exp" > exp.diff; then
        success "* $(basename "$exp")"
      else
        cat exp.diff
        fatal "* $(basename "$exp")"
      fi
    fi
  fi
done
popd > /dev/null

info "Cleaning up temp files"
rm -r "$target"

success "Test passed"
