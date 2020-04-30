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
# --- end runfiles.bash initialization --- }}}

# Find logging and hermetic_tar with rlocation, as this script is run from a genrule

# shellcheck disable=SC1090
source "$(rlocation com_stripe_sorbet_llvm/test/logging.sh)"

# shellcheck disable=SC1090
source "$(rlocation com_stripe_sorbet_llvm/test/hermetic_tar.sh)"

# Argument Parsing #############################################################

# Positional arguments
output_archive=$1
shift 1

# Sources make up the remaining input
ruby_source=( "$@" )

# Environment Setup ############################################################

sorbet="$(rlocation com_stripe_sorbet_llvm/main/sorbet)"

# The script is always run from the repo root
root="$PWD"

# Temporary directory for llvm output
target="$(mktemp -d)"

output=$(mktemp)

# Main #########################################################################

indent_and_nest() {
  sed -e 's/^/       │/'
}

echo
info "Input(s):"
i=1
for file in "${ruby_source[@]}"; do
  if [ "$i" -ne "${#ruby_source[@]}" ]; then
    info "├─ $file"
  else
    info "└─ $file"
  fi
  (( i++ ))
done

echo
info "Using Sorbet to generate llvm-ir-folder..."
llvm_ir_folder_flag="--llvm-ir-folder=$target"
info "├─ Using $llvm_ir_folder_flag"
set +e
$sorbet --silence-dev-message --no-error-count \
  "$llvm_ir_folder_flag" "${ruby_source[@]}" > "$output" 2>&1
echo "$?" > "$target/sorbet.exit"
set -e
success "└─ done."

echo
info "Building tar archive..."
cp "$output" "$target/sorbet.outerr"
hermetic_tar "$target" "$root/$output_archive"

success "└─ done."
echo

# vim:fdm=marker
