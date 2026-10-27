#!/usr/bin/env bash
set -u

sorbet="$PWD/main/sorbet"
cd test/cli/focus-packages || exit 1

# Imports point from consumer to dependency:
#   Top -> Consumer -> Target -> Lib::Base -> RootDep
#   Consumer -> Extra -> ExtraDep
#   Target::Test -> Target, TestSupport
#   TestTop -> TestConsumer -> Target
#   CycleA <-> CycleB, CycleA -> Target
#   Sibling -> Lib::Base; Other is disconnected.
# Top::Test, Target::Test, TestConsumer, and TestTop are explicit test packages.

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

echo '--- Target: includes dependencies and dependents ---'
run --stripe-packages --focus-packages=Target

echo '--- Target: skipping dependents, including explicit test consumers ---'
run --stripe-packages --focus-packages=Target --focus-skip-downstream

echo '--- Target: skipping inference for dependencies ---'
run --stripe-packages --focus-packages=Target --focus-skip-upstream

echo '--- Target: skipping dependents and inference for dependencies ---'
run --stripe-packages --focus-packages=Target --focus-skip-upstream --focus-skip-downstream

echo '--- Multiple names, repeated flag, and duplicate roots ---'
run --sorbet-packages --focus-packages=Target,Sibling --focus-packages=Target

echo '--- Namespaced package ---'
run --stripe-packages --focus-packages=Lib::Base

echo '--- Cycle: dependencies do not pull in their other consumers ---'
run --stripe-packages --focus-packages=CycleB

echo '--- Test support selection includes explicit test consumers ---'
run --stripe-packages --focus-packages=TestSupport

echo '--- Focused root includes explicit test consumers ---'
run --stripe-packages --focus-packages=Top

echo '--- Explicitly selected tests retain their dependencies ---'
run --stripe-packages --focus-packages=Top::Test

cache_dir=$(mktemp -d)
trap 'rm -rf "$cache_dir"' EXIT

echo '--- Cold cache ---'
run --stripe-packages --cache-dir="$cache_dir" --focus-packages=Target

echo '--- Warm cache, different selection ---'
run --stripe-packages --cache-dir="$cache_dir" --focus-packages=Sibling

echo '--- Warm cache, original selection ---'
run --stripe-packages --cache-dir="$cache_dir" --focus-packages=Target

echo '--- Full check after caching partial checks ---'
run --stripe-packages --cache-dir="$cache_dir"

echo '--- Unknown package ---'
run --stripe-packages --focus-packages=Target,Missing

echo '--- Namespace is not itself a package ---'
run --stripe-packages --focus-packages=Lib

echo '--- Empty selection ---'
run --stripe-packages --focus-packages=

echo '--- Empty name in selection ---'
run --stripe-packages --focus-packages=Target,,Sibling

echo '--- Package mode required ---'
run --focus-packages=Target

echo '--- Package-directed mode required with either package flag ---'
run --stripe-packages --experimental-package-directed=false --focus-packages=Target
run --sorbet-packages --experimental-package-directed=false --focus-packages=Target

echo '--- LSP requires the whole project ---'
run --stripe-packages --focus-packages=Target --lsp

echo '--- Stored state requires the whole project ---'
run --stripe-packages --focus-packages=Target --store-state=gs,files,names

echo '--- Package generation requires the whole project ---'
run --stripe-packages --focus-packages=Target --gen-packages
