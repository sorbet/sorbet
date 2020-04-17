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

llvm_diff_path=$root/external/llvm_toolchain/bin/llvm-diff
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
    actual="$target/${rb[0]}.$ext"
    if [ ! -f "$actual" ]; then
      fatal "No LLVMIR found at" "$actual"
    fi
    if [[ "$OSTYPE" != "darwin"* ]]; then
      diff_out="${diff_dir}/${ext}.diff"
      if $llvm_diff_path "$exp" "$actual" > "$diff_out" 2>&1 ; then
        if grep "exists only in" "$diff_out" > /dev/null ; then
          cat "$diff_out"
          fatal "* $(basename "$exp")"
        else
          success "* $(basename "$exp")"
        fi
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
