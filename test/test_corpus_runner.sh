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
rbcode=$(< "$rbexit")
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

echo ""
attn "Troubleshooting? Use these helpers interactively at the shell:"
info "├─ test/run_ruby.sh ${rb[0]}"
info "├─ test/run_sorbet.sh ${rb[0]}"
info "└─ test/run_compiled.sh ${rb[0]}"
info ""
attn "Or these to attach a debugger:"
info "├─ test/run_ruby.sh -d ${rb[0]}"
info "├─ test/run_sorbet.sh -d ${rb[0]}"
info "└─ test/run_compiled.sh -d ${rb[0]}"

echo ""
info "Testing ruby..."
if ! $ruby -e 'puts (require "set")' > /dev/null; then
  fatal "└─ Ruby is not functioning:  bazel-bin/$ruby"
else
  success "└─ path:        bazel-bin/$ruby"
fi

echo ""
info "Pre-computed output of running interpreted:"
info "├─ stdout:      bazel-out/k8-opt/bin/$rbout"
info "├─ stderr:      bazel-out/k8-opt/bin/$rberr"
info "└─ exit code:   $rbcode"

indent_and_nest() {
  sed -e 's/^/       │/'
}

echo    ""
info    "Unpacking compiled artifact (.so/.bundle, .ll, .llo)..."
info    "├─ from:        bazel-out/k8-opt/bin/${build_archive}"
tar -xf "${build_archive}" -C "${target}"
info    "├─ contents:"
find "$target" -type f | indent_and_nest
success "└─ done."

# NOTE: running the test could be split out into its own genrule, the test just
# needs to validate that the output matches.
echo ""
info "Running compiled version with preamble..."

# NOTE: using a temp file here, as that will cause ruby to not print the name of
# the main file in a stack trace.
echo "require './$rbmain'" > "$runfile"

set +e
# NOTE: the llvmir environment variable must have a leading `./`, otherwise the
# require will trigger path search.
force_compile=1 llvmir="${target}" $ruby \
  --disable=gems \
  --disable=did_you_mean \
  -r "${root}/external/com_stripe_ruby_typer/gems/sorbet-runtime/lib/sorbet-runtime.rb" \
  -r "${root}/test/patch_require.rb" \
  "$runfile" \
  1> "$stdout" 2> "$stderr"
code=$?
set -e

info    "├─ stdout:      $stdout"
info    "├─ stderr:      $stderr"
success "└─ exit code:   $code"

shorten_bazel() {
  sed -e "s+_bazel_$USER/[^ ]*com_stripe_sorbet_llvm/+bazel/.../com_stripe_sorbet_llvm/+"
}

should_fail=

echo ""
info "Checking return codes match..."
if [[ "$code" != "$rbcode" ]]; then
  error "├─ return codes don't match."
  error "├─ Ruby:     ${rbcode}"
  error "└─ Compiled: ${code}"
  should_fail=1
else
  success "└─ codes match."
fi

echo ""
info "Checking stdouts match..."
if ! diff -au "$rbout" "$stdout" > stdout.diff; then
  attn  "├─ Diff (interpreted vs compiled)"
  < stdout.diff indent_and_nest
  info  "├─ stdout (interpreted)"
  < "$rbout" shorten_bazel | indent_and_nest
  info  "├─ stdout (compiled)"
  < "$stdout" shorten_bazel | indent_and_nest
  error "└─ stdouts don't match. See above."
  should_fail=1
else
  success "└─ stdouts match."
fi


echo ""
info "Checking stderrs match..."

filter_stderr() {
  sed -e '/^SorbetLLVM using compiled/d' | \
    shorten_bazel | \
    sed -e 's+/[^ ]*/tmp\.[[:alnum:]]*+/.../tmp.XXXXXXXXXX+'
}

if grep -q '^# skip_stderr_check$' "$rbmain"; then
  attn "└─ skipping stderr check."
else
  stderr_filtered=$(mktemp)
  rberr_filtered=$(mktemp)
  filter_stderr < "$rberr" > "$rberr_filtered"
  filter_stderr < "$stderr" > "$stderr_filtered"
  if ! diff -au "$rberr_filtered" "$stderr_filtered" > stderr.diff; then
    attn  "├─ Diff (interpretted vs compiled, filtered):"
    < stderr.diff indent_and_nest
    attn  "├─ Full compiled output (except bazel paths shortened):"
    < "$stderr" shorten_bazel | indent_and_nest
    error "└─ stderrs don't match. See above."
    should_fail=1
  else
    success "└─ stderrs match."
  fi
fi

if [ -z "$should_fail" ]; then
  echo ""
  success "Test passed."
  info    "├─ stdout (interpretted):"
  < "$rbout" indent_and_nest
  info    "├─ stderr (interpretted):"
  < "$rberr" shorten_bazel | indent_and_nest
  success "└─ 🎉"

  echo ""
else
  echo ""
  error "Test failed."
  echo ""
  exit 1
fi
