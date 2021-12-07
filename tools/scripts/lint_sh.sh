#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

list_sh_src() {
    git ls-files -z -c -m -o --exclude-standard -- '*.sh' '*.bash'
}


unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

# Fetches shellcheck binary
if ! bazel build "@shellcheck_$platform//:shellcheck_exe"; then
  echo "warning: Could not fetch shellcheck with Bazel. Falling back to system"
  shellcheck="shellcheck"
else
  shellcheck="bazel-sorbet/external/shellcheck/shellcheck"
fi

if [ "$1" = "-t" ]; then
  OUTPUT=$(list_sh_src | xargs -0 "$shellcheck" -s bash || :)
  if [ -n "$OUTPUT" ]; then
    echo "Some shell files have lint errors!"
    echo ""
    echo -n "\`\`\`"
    echo "$OUTPUT"
    echo "\`\`\`"
    echo ''
    echo "Run \`./tools/scripts/lint_sh.sh\` to see the errors."
    exit 1
  fi
else
    list_sh_src | xargs -0 "$shellcheck" -s bash
fi
