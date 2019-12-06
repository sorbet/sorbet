#!/bin/bash
set -euo pipefail

# This script is always run from the repo root, so `logging.sh` doesn't exist
# where shellcheck thinks it does.
# shellcheck disable=SC1091
source "test/logging.sh"

root="$PWD"

# Positional arguments
rbout=${1/--expected_output=/}
rbexit=${2/--expected_exit_code=/}
build_archive=${3/--build_archive=/}
ruby=${4/--ruby=/}
shift 4

# sources make up the remaining argumenets
rbmain=$1
rb=( "$@" )

info "--- Debugging ---"
info "* Run ruby locally"
for source in "${rb[@]}"; do
  attn "    test/run_test.sh $source"
done

info "* Run sorbet"
for source in "${rb[@]}"; do
  attn "    test/run_sorbet.sh $source"
done

info "* Debug compiled code"
for source in "${rb[@]}"; do
  attn "    test/debug_compiled.sh $source"
done

info "--- Test Config ---"
info "* Source: ${rb[*]}"
info "* Oracle: ${rbout}"
info "* Exit:   ${rbexit}"
info "* Build:  ${build_archive}"
info "* Ruby:   ${ruby}"

info "--- Testing ruby ---"
$ruby -e 'puts (require "set")' > /dev/null || fatal "No functioning ruby"

info "--- Unpacking Build ---"
tar -xvf "${build_archive}"

# NOTE: exp file validation could be its own test, which would allow it to
# execute in parallel with the oracle verification.
info "--- Checking Build ---"
for ext in "llo"; do
  exp=${source%.rb}.$ext.exp
  if [ -f "$exp" ]; then
    actual=("target/"*".$ext")
    if [ ! -f "${actual[0]}" ]; then
      fatal "No LLVMIR found at" "${actual[@]}"
    fi
    if [[ "$OSTYPE" == "darwin"* ]]; then
      if diff -u <(grep -v '^target triple =' < "${actual[@]}") "$exp" > exp.diff; then
        success "* $exp"
      else
        cat exp.diff
        fatal "* $exp"
      fi
    fi
  fi
done

# NOTE: running the test could be split out into its own genrule, the test just
# needs to validate that the output matches.
info "--- Running Compiled Test ---"

# NOTE: using a temp file here, as that will cause ruby to not print the name of
# the main file in a stack trace.
runfile=$(mktemp)
echo "require './$rbmain'" > "$runfile"

cat "$runfile"

set +e
# NOTE: the llvmir environment variable must have a leading `./`, otherwise the
# require will trigger path search.
force_compile=1 llvmir="./target" $ruby \
  -I "${root}/run/tools" \
  -rpatch_require.rb -rpreamble.rb "$runfile" \
  2> stderr.log | tee stdout.log
code=$?
set -e

info "--- Checking Return Code ---"
rbcode=$(cat "$rbexit")
if [[ "$code" != "$rbcode" ]]; then
  info "* Stdout"
  cat stdout.log
  info "* Stderr"
  cat stderr.log

  error "Return codes don't match"
  error "  * Ruby:     ${rbcode}"
  fatal "  * Compiled: ${code}"
fi

info "--- Checking Stdout ---"
if ! diff -au stdout.log "$rbout" > stdout.diff; then
  error "* Stdout diff"
  cat stdout.diff
  info  "* Stderr"
  cat  stderr.log
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

success "Test passed"
