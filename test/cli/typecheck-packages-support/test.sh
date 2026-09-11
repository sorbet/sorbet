#!/usr/bin/env bash
set -u

sorbet="$PWD/main/sorbet"
cd test/cli/typecheck-packages-support || exit 1

run() {
  local output status
  # Permit the unpackaged global RBI so this fixture can check that its definitions remain available.
  output=$("$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
    --no-error-sections --suppress-error-code=3705 --stripe-packages \
    --extra-package-files-directory-prefix-underscore=./generated/ \
    "$@" . 2>&1)
  status=$?
  printf '%s\n' "$output" | sed '/^$/d; /^[[:space:]]/d' | LC_ALL=C sort
  echo "exit: $status"
}

echo '--- Nested package keeps global RBIs, generated RBIs, and preludes ---'
run --typecheck-packages=App::Isolated

echo '--- The same selection in package-directed mode ---'
run --typecheck-packages=App::Isolated --experimental-package-directed

echo '--- Explicit test package is a consumer ---'
run --typecheck-packages=App

echo '--- Selecting the test package includes application dependencies ---'
run --typecheck-packages=App::Test

echo '--- Explicit test package mode ---'
run --typecheck-packages=App --experimental-test-packages --experimental-package-directed

echo '--- Selecting a prelude selects all of its implicit consumers ---'
run --typecheck-packages=Prelude

echo '--- Selecting a prelude in package-directed mode ---'
run --typecheck-packages=Prelude --experimental-package-directed

echo '--- File table omits unread sources ---'
output=$("$sorbet" --silence-dev-message --max-threads=0 --suppress-error-code=3705 \
  --stripe-packages --extra-package-files-directory-prefix-underscore=./generated/ \
  --typecheck-packages=App::Isolated --print=file-table-json . 2>&1)
status=$?
printf '%s\n' "$output" | sed -n '/"path":/p' | LC_ALL=C sort
echo "exit: $status"

echo '--- Typed-sigil suggestions stay within the selection ---'
# --suggest-typed disallows --suppress-error-code, so invoke Sorbet directly.
output=$("$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
  --stripe-packages --extra-package-files-directory-prefix-underscore=./generated/ \
  --typecheck-packages=App::Isolated --suggest-typed --typed=strict --isolate-error-code=7022 \
  --no-error-sections . 2>&1)
status=$?
printf '%s\n' "$output" | sed '/^$/d; /^[[:space:]]/d' | LC_ALL=C sort
echo "exit: $status"
