#!/usr/bin/env bash
set -u

sorbet="$PWD/main/sorbet"
cd test/cli/typecheck-packages || exit 1

# Imports point from consumer to dependency:
#   Top -> Consumer -> Target -> Lib::Base -> RootDep
#   Consumer -> Extra -> ExtraDep
#   Target --test_import--> TestSupport
#   TestTop --test_import (test_rb)--> TestConsumer --test_import--> Target
#   CycleA <-> CycleB, CycleA -> Target
#   Sibling -> Lib::Base; Other is disconnected.
# Top has an explicit test package; its dependencies still use implicit tests.

run() {
  local output status
  output=$("$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
    --no-error-sections --no-error-count --experimental-package-directed "$@" . 2>&1)
  status=$?
  # Keep diagnostic headlines; source excerpts do not help distinguish the selected packages.
  printf '%s\n' "$output" | sed '/^$/d; /^[[:space:]]/d' | LC_ALL=C sort
  echo "exit: $status"
}

echo '--- All packages (including unrelated syntax and resolution errors) ---'
run --stripe-packages

echo '--- Target: production dependencies exclude consumers and their dependencies ---'
run --stripe-packages --typecheck-packages=Target

echo '--- Same selection with worker threads ---'
run --stripe-packages --max-threads=2 --typecheck-packages=Target

echo '--- Multiple names, repeated flag, and duplicate roots ---'
run --sorbet-packages --typecheck-packages=Target,Sibling --typecheck-packages=Target

echo '--- Namespaced package ---'
run --stripe-packages --typecheck-packages=Lib::Base

echo '--- Cycle: dependencies do not pull in their other consumers ---'
run --stripe-packages --typecheck-packages=CycleB

echo '--- Production selection does not follow test-only consumers ---'
run --stripe-packages --typecheck-packages=TestSupport

echo '--- Migrated root does not select implicit tests of dependencies ---'
run --stripe-packages --typecheck-packages=Top

echo '--- Explicitly selected tests retain their dependencies ---'
run --stripe-packages --typecheck-packages=Top::Test

cache_dir=$(mktemp -d)
trap 'rm -rf "$cache_dir"' EXIT

echo '--- Cold cache ---'
run --stripe-packages --cache-dir="$cache_dir" --typecheck-packages=Target

echo '--- Warm cache, different selection ---'
run --stripe-packages --cache-dir="$cache_dir" --typecheck-packages=Sibling

echo '--- Warm cache, original selection ---'
run --stripe-packages --cache-dir="$cache_dir" --typecheck-packages=Target

echo '--- Full check after caching partial checks ---'
run --stripe-packages --cache-dir="$cache_dir"

echo '--- Unknown package ---'
run --stripe-packages --typecheck-packages=Target,Missing

echo '--- Namespace is not itself a package ---'
run --stripe-packages --typecheck-packages=Lib

echo '--- Empty selection ---'
run --stripe-packages --typecheck-packages=

echo '--- Empty name in selection ---'
run --stripe-packages --typecheck-packages=Target,,Sibling

echo '--- Package mode required ---'
run --typecheck-packages=Target

echo '--- Package-directed mode required with either package flag ---'
run --stripe-packages --experimental-package-directed=false --typecheck-packages=Target
run --sorbet-packages --experimental-package-directed=false --typecheck-packages=Target

echo '--- LSP requires the whole project ---'
run --stripe-packages --typecheck-packages=Target --lsp

echo '--- Stored state requires the whole project ---'
run --stripe-packages --typecheck-packages=Target --store-state=gs,files,names

echo '--- Package generation requires the whole project ---'
run --stripe-packages --typecheck-packages=Target --gen-packages
