#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")/../.."

tmp_output="$(mktemp)"
# shellcheck disable=SC2064
trap "rm -rf '$tmp_output'" EXIT

indent4() {
  sed -e 's/^/    /'
}

grep -r --only-matching '^constexpr ErrorClass [^{]*{\([0-9]*\),' core/errors/ | \
    grep --only-matching '[0-9]*' | \
    sort | \
    uniq -c > "$tmp_output"

had_error=0
if grep -q '^ *[2-9]' "$tmp_output"; then
  echo "error: Some error codes in core/errors/ are duplicated:"
  echo

  while IFS= read -r error_code
  do
    grep -r -n "$error_code" core/errors | indent4
  done < <(grep '^ *[2-9]' "$tmp_output" | awk '{ print $2 }')

  echo
  echo "Please manually fix these duplicated codes."

  had_error=1
fi

while IFS= read -r error_code
do
  if [[ "$error_code" = 5* ]]; then
    # TODO(jez) Document all other error codes
    continue
  fi

  if ! grep -q "^## $error_code\$" website/docs/error-reference.md; then
    echo "error: The error error '$error_code' in core/errors/ is not documented in website/docs/error-reference.md. Defined here:"
    grep -r -n "$error_code" core/errors | indent4
    had_error=1
  fi
done < <(awk '{ print $2 }' < "$tmp_output")

exit "$had_error"
