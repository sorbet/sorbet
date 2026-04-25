#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")/../.."

list_sh_src() {
  git ls-files -z -c -m -o --exclude-standard -- '*.sh' '*.bash'
}

kernel_name="$(uname -s | tr '[:upper:]' '[:lower:]')"
march="$(uname -m)"
shellcheck_pkg="shellcheck_${kernel_name}_${march}"

# Fetches shellcheck binary
bazel_build="$(mktemp)"
trap 'rm -f "$bazel_build"' EXIT
if ! bazel build "@$shellcheck_pkg//:shellcheck_exe" &> "$bazel_build"; then
  echo "warning: Could not fetch shellcheck with Bazel. Falling back to system"
  echo '```'
  cat "$bazel_build"
  echo '```'
  shellcheck="shellcheck"
else
  shellcheck="$(bazel info output_base 2> /dev/null)/external/$shellcheck_pkg/shellcheck"
fi

"$shellcheck" --version

if [ "${1:-}" = "-t" ]; then
  if ! output=$(list_sh_src | xargs -0 "$shellcheck" -s bash 2>&1); then
    echo ""
    echo "Some shell files have lint errors!"
    echo ""
    echo -n "\`\`\`"
    echo "$output"
    echo "\`\`\`"
    echo ''
    echo "Run \`./tools/scripts/lint_sh.sh\` to see the errors."
    exit 1
  fi
else
  list_sh_src | xargs -0 "$shellcheck" -s bash
fi
