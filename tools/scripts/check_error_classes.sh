#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")/../.."

tmp_output="$(mktemp)"
# shellcheck disable=SC2064
trap "rm -rf '$tmp_output'" EXIT

grep -r --only-matching '^constexpr ErrorClass [^{]*{\([0-9]*\),' core/errors/ | \
    grep --only-matching '[0-9]*' | \
    sort | \
    uniq -c > "$tmp_output"

if grep -q '^ *[2-9]' "$tmp_output"; then
  echo "error: Some error codes in core/errors/ are duplicated:"
  echo

  while IFS= read -r error_code
  do
    grep -r -n "$error_code" core/errors | sed -e 's/^/    /'
  done < <(grep '^ *[2-9]' "$tmp_output" | awk '{ print $2 }')

  echo
  echo "Please manually fix these duplicated codes."

  exit 1
fi
