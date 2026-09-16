#!/usr/bin/env bash
set -u

sorbet="$PWD/main/sorbet"
cd test/cli/typecheck-packages-invalid-imports || exit 1

# Invalid imports must not crash graph traversal. Package declarations are still
# validated globally, while ordinary source errors are limited to the selection.
echo '--- Invalid imports in selected and unselected packages ---'
output=$("$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
  --no-error-sections --no-error-count --stripe-packages --experimental-package-directed \
  --typecheck-packages=App . 2>&1)
status=$?
printf '%s\n' "$output" | sed '/^$/d; /^[[:space:]]/d' | LC_ALL=C sort
echo "exit: $status"
