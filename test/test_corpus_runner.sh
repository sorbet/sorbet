#!/bin/bash
set -euo pipefail

# This script is always run from the repo root, so `logging.sh` doesn't exist
# where shellcheck thinks it does.
# shellcheck disable=SC1091
source "test/logging.sh"

# Argument Parsing #############################################################

# Positional arguments
rbout=${1/--expected_output=/}

# TODO(trevor): Remove this once stderr is being checked
# shellcheck disable=SC2034
rberr=${2/--expected_err=/}
rbexit=${3/--expected_exit_code=/}
build_archive=${4/--build_archive=/}
ruby=${5/--ruby=/}
shift 5

# sources make up the remaining argumenets
rbmain=$1
rb=( "$@" )

# Environment Setup ############################################################

root="$PWD"

# The directory to unpack the build archive to
target="$(mktemp -d)"

# Test stdout/stderr logs
stdout="$(mktemp)"
stderr="$(mktemp)"

# Test wrapper
runfile="$(mktemp)"

# Main #########################################################################

info "--- Running ---"
info "* Ruby interpreted"
attn "    test/run_ruby.sh ${rb[0]}"

info "* Sorbet compiler"
attn "    test/run_sorbet.sh ${rb[0]}"

info "* Compiled code"
attn "    test/run_compiled.sh ${rb[0]}"

info "--- Debugging ---"
info "* Ruby interpreted"
attn "    test/run_ruby.sh -d ${rb[0]}"

info "* Sorbet compiler"
attn "    test/run_sorbet.sh -d ${rb[0]}"

info "* Compiled code"
attn "    test/run_compiled.sh -d ${rb[0]}"


info "--- Test Config ---"
info "* Source: ${rb[*]}"
info "* Oracle: bazel-genfiles/${rbout}"
info "* Exit:   bazel-genfiles/${rbexit}"
info "* Build:  bazel-genfiles/${build_archive}"
info "* Ruby:   bazel-bin/${ruby}"
info "* Target: ${target}"
info "* Runfile:${runfile}"
info "* Stderr: ${stderr}"
info "* Stdout: ${stderr}"

info "--- Testing ruby ---"
$ruby -e 'puts (require "set")' > /dev/null || fatal "No functioning ruby"

info "--- Unpacking Build ---"
tar -xvf "${build_archive}" -C "${target}"

# NOTE: running the test could be split out into its own genrule, the test just
# needs to validate that the output matches.
info "--- Running Compiled Test ---"

# NOTE: using a temp file here, as that will cause ruby to not print the name of
# the main file in a stack trace.
echo "require './$rbmain'" > "$runfile"

set +e
# NOTE: the llvmir environment variable must have a leading `./`, otherwise the
# require will trigger path search.
force_compile=1 llvmir="${target}" $ruby \
  -I "${root}/run/tools" \
  -rpatch_require.rb -rpreamble.rb "$runfile" \
  2> "$stderr" | tee "$stdout"
code=$?
set -e

info "--- Checking Return Code ---"
rbcode=$(cat "$rbexit")
if [[ "$code" != "$rbcode" ]]; then
  info "* Stdout"
  cat "$stdout"
  info "* Stderr"
  cat "$stderr"

  error "Return codes don't match"
  error "  * Ruby:     ${rbcode}"
  fatal "  * Compiled: ${code}"
fi

info "--- Checking Stdout ---"
if ! diff -au "$rbout" "$stdout" > stdout.diff; then
  error "* Stdout diff"
  cat stdout.diff
  info  "* Stderr"
  cat  "$stderr"
  fatal
fi

# info "--- Checking Stderr ---"
# if ! diff -au stderr.log "$rberr" > stderr.diff; then
#   error "* Stderr diff"
#   cat stderr.diff
#   info  "* Stderr"
#   cat  stderr.log
#   fatal
# fi

info "Cleaning up temp files"
rm -r "$target" "$stdout" "$stderr" "$runfile"

success "Test passed"
