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
  # shellcheck disable=SC1091
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

# shellcheck source=SCRIPTDIR/logging.sh
source "$(rlocation com_stripe_ruby_typer/test/logging.sh)"

# shellcheck source=SCRIPTDIR/hermetic_tar.sh
source "$(rlocation com_stripe_ruby_typer/test/hermetic_tar.sh)"

# Argument Parsing #############################################################

# Positional arguments
output_dir=$1
stdout=$2
exitcode=$3
shift 3

# Sources make up the remaining input
ruby_source=( "$@" )

# Environment Setup ############################################################

sorbet="$(rlocation com_stripe_ruby_typer/compiler/sorbet)"

# Main #########################################################################

if [ ! -d "$output_dir" ]; then
  info "Creating output dir $output_dir"
  mkdir "$output_dir"
fi

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
info "Using Sorbet to generate compiled files and llvm ir..."
compiled_out_dir_flag="--compiled-out-dir=$output_dir"
llvm_ir_dir_flag="--llvm-ir-dir=$output_dir"
info "├─ Using $compiled_out_dir_flag"
info "├─ Using $llvm_ir_dir_flag"
set +e
$sorbet --silence-dev-message --no-error-count \
  "$compiled_out_dir_flag" "$llvm_ir_dir_flag" "${ruby_source[@]}" > "$stdout" 2>&1
echo "$?" > "$exitcode"
set -e

success "└─ done."

# vim:fdm=marker
