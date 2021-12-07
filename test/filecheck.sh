#!/bin/bash

set -euo pipefail

# shellcheck source-path=SCRIPTDIR/..
source "test/logging.sh"

# Argument Parsing #############################################################

# Positional arguments
build_dir=${1/--build_dir=/}
shift 1

# sources make up the remaining argumenets
rb=( "$@" )

# Environment Setup ############################################################

root=$PWD

filecheck_path="${root}/external/llvm/FileCheck"

diff_dir="$(mktemp -d)"

cleanup() {
  rm -rf "$diff_dir"
}
trap cleanup EXIT

# Main #########################################################################

info "Checking Build:"
pushd "$build_dir/" > /dev/null
for pass in "INITIAL" "LOWERED" "OPT"; do
    case "$pass" in
        INITIAL)
            output="${rb[0]}.ll"
            ;;
        LOWERED)
            output="${rb[0]}.lowered.ll"
            ;;
        OPT)
            output="${rb[0]}.opt.ll"
            ;;
        *)
            fatal "Don't know how to examine $pass"
            ;;
    esac
    if grep -q "# run_filecheck: $pass" "$root/${rb[0]}" 2>/dev/null; then
        if $filecheck_path "$root/${rb[0]}" -check-prefix="$pass" --input-file "$output" 2>&1 ; then
            success "* ${rb[0]} FileCheck $pass"
        else
            fatal "* ${rb[0]} FileCheck $pass"
        fi
    fi
done
popd > /dev/null

success "Test passed"
