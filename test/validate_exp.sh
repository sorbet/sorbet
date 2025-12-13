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

# Positional arguments
build_dir=${1/--build_dir=/}
expected_failure=
case "${2/--expected_failure=/}" in
  True) expected_failure=1;;
  False);;
  *) fatal "Expected --expected_failure=(True|False), got $2";;
esac
shift 2

# sources make up the remaining argumenets
rb=( "$@" )

# Environment Setup ############################################################

root=$PWD

llvm_diff_path="$root/external/llvm_toolchain_15_0_7_llvm/bin/llvm-diff"

ruby="$(rlocation sorbet_ruby_2_7_for_compiler/ruby)"
diff_diff="$(rlocation com_stripe_ruby_typer/test/diff-diff.rb)"

diff_dir="$(mktemp -d)"

cleanup() {
  rm -rf "$diff_dir"
}
trap cleanup EXIT

# Main #########################################################################

info "Checking Build:"
pushd "$build_dir/" > /dev/null

something_failed() {
  if [ -n "${expected_failure}" ]; then
    success "Disabled test failed as expected."
    info    "* $1"
    echo ""
    info    "To make this failing test fail the build, move it out of the disabled folder"
    echo ""
    exit 0
  else
    echo ""
    error "Test failed:"
    error "* $1"
    echo ""
    exit 1
  fi
}

exts=("opt.ll")
for ext in "${exts[@]}"; do
  exp="$root/${rb[0]%.rb}.$ext.exp"
  if [ -f "$exp" ]; then
    actual="${rb[0]}.$ext"
    if [ ! -f "$actual" ]; then
      fatal "No LLVMIR found at" "$actual"
    fi
    if [[ "$OSTYPE" != "darwin"* ]]; then
      diff_out="${diff_dir}/${ext}.diff"

      # NOTE: because of https://bugs.llvm.org/show_bug.cgi?id=48137, we pipe
      # the output of llvm-diff through the diff-diff.rb script to clean out
      # spurious errors from its output. This will also take over returning the
      # correct exit code if it discovers that all of the differences were
      # related to the bug.
      if ($llvm_diff_path "$exp" "$actual" 2>&1 || true) | "$ruby" "$diff_diff" > "$diff_out" ; then
        if grep "exists only in" "$diff_out" > /dev/null ; then
          cat "$diff_out"
          info "If this was an expected difference, you need to run tools/scripts/update_compiler_exp.sh"
          info "(or in CI: you can find the new LLVM IR in the Artifacts tab)"
          info "Actual LLVM output: $build_dir/$actual"
          something_failed "$(basename "$exp")"
        else
          if [ -n "${expected_failure}" ]; then
            echo ""
            error "Disabled test did not fail."
            info   "This could mean that a recent change has made this test start passing."
            info   "If that's the case, great! Please move this test out of the disabled folder to catch future regressions."
            echo ""
            exit 1
          else
            success "* $(basename "$exp")"
          fi
        fi
      else
        cat "$diff_out"
        info "If this was an expected difference, you need to run tools/scripts/update_compiler_exp.sh"
        info "(or in CI: you can find the new LLVM IR in the Artifacts tab)"
        info "Actual LLVM output: $build_dir/$actual"
        something_failed "$(basename "$exp")"
      fi
    fi
  fi
done
popd > /dev/null

success "Test passed"
