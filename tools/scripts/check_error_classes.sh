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
  if ! grep -q "^## $error_code\$" website/docs/error-reference.md; then
    echo "error: The error error '$error_code' in core/errors/ is not documented in website/docs/error-reference.md. Defined here:"
    grep -r -n "$error_code" core/errors | indent4
    had_error=2
  fi
done < <(awk '{ print $2 }' < "$tmp_output")

if [ "$had_error" = 2 ]; then
  echo "When documenting errors in the error reference, think outside the box."
  echo "Think about how the error might be confusing *in practice*, and document"
  echo "*that* example in context. The error reference is a chance to give error"
  echo "messages that would otherwise be hard to produce, as well as additional"
  echo "context, especially for new Sorbet users."
fi

exit "$had_error"
