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
# Prelude is an implicit dependency of every package. It patches String with a method used by Target;
# the method's inference error should disappear only when upstream inference is skipped in focused mode.
# Files in generated/ follow their package's selection despite living outside its normal directory.

run() {
  local output status
  output=$("$sorbet" --silence-dev-message --censor-for-snapshot-tests --max-threads=0 \
    --no-error-sections --no-error-count --experimental-package-directed \
    --extra-package-files-directory-prefix-slash=./generated/ "$@" . 2>&1)
  status=$?
  # Keep diagnostic headlines; source excerpts do not help distinguish the selected packages.
  printf '%s\n' "$output" | sed '/^$/d; /^[[:space:]]/d' | LC_ALL=C sort
  echo "exit: $status"
}

echo '--- All packages (including unrelated syntax and resolution errors) ---'
run --stripe-packages

echo '--- Prelude selection checks all packages ---'
run --stripe-packages --focus-packages=Prelude

echo '--- Prelude selection checks all packages with downstream skipping ---'
run --stripe-packages --focus-packages=Prelude --focus-skip-downstream

echo '--- Prelude selection checks all packages with upstream skipping ---'
run --stripe-packages --focus-packages=Prelude --focus-skip-upstream

echo '--- Prelude selection checks all packages with both skip flags ---'
run --stripe-packages --focus-packages=Prelude --focus-skip-downstream --focus-skip-upstream

echo '--- Target: includes dependencies and dependents ---'
run --stripe-packages --focus-packages=Target

echo '--- Target: skipping dependents, including explicit test consumers ---'
run --stripe-packages --focus-packages=Target --focus-skip-downstream

echo '--- Target: skipping inference for dependencies ---'
run --stripe-packages --focus-packages=Target --focus-skip-upstream

echo '--- Target: skipping dependents and inference for dependencies ---'
run --stripe-packages --focus-packages=Target --focus-skip-upstream --focus-skip-downstream

# Split every non-prelude stratum into SCCs, then separately combine the entire selection into one stratum.
echo '--- Target: split strata preserve the selection ---'
run --stripe-packages --focus-packages=Target --stratum-split-threshold=1

echo '--- Target: combined strata preserve the selection ---'
run --stripe-packages --focus-packages=Target --min-stratum-size=1000

# A smaller minimum combines only some of the split strata. Selection and inference skipping must stay the same.
echo '--- Target: split and combine strata ---'
run --stripe-packages --focus-packages=Target --stratum-split-threshold=1 --min-stratum-size=5

echo '--- Target: split and combine strata, skipping dependents ---'
run --stripe-packages --focus-packages=Target --stratum-split-threshold=1 --min-stratum-size=5 --focus-skip-downstream

echo '--- Target: split and combine strata, skipping inference for dependencies ---'
run --stripe-packages --focus-packages=Target --stratum-split-threshold=1 --min-stratum-size=5 --focus-skip-upstream

echo '--- Target: split and combine strata with both skip flags ---'
run --stripe-packages --focus-packages=Target --stratum-split-threshold=1 --min-stratum-size=5 \
  --focus-skip-upstream --focus-skip-downstream

echo '--- Multiple names, repeated flag, and duplicate roots ---'
run --sorbet-packages --focus-packages=Target,Sibling --focus-packages=Target

echo '--- Explicitly selected dependencies are inferred with both skip flags ---'
run --stripe-packages --focus-packages=Target,Lib::Base --focus-skip-upstream --focus-skip-downstream

echo '--- Namespaced package ---'
run --stripe-packages --focus-packages=Lib::Base

echo '--- Cycle: dependencies do not pull in their other consumers ---'
run --stripe-packages --focus-packages=CycleB

echo '--- Cycle: both members are inferred with both skip flags ---'
run --stripe-packages --focus-packages=CycleB --focus-skip-downstream --focus-skip-upstream

echo '--- Test support selection includes explicit test consumers ---'
run --stripe-packages --focus-packages=TestSupport

echo '--- Focused root includes explicit test consumers ---'
run --stripe-packages --focus-packages=Top

echo '--- Explicitly selected tests retain their dependencies ---'
run --stripe-packages --focus-packages=Top::Test

echo '--- A clean focused package succeeds despite errors elsewhere ---'
run --stripe-packages --focus-packages=Top::Test --focus-skip-upstream --focus-skip-downstream

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

echo '--- Unknown package after a prelude ---'
run --stripe-packages --focus-packages=Prelude,Missing

echo '--- Unknown package before a prelude ---'
run --stripe-packages --focus-packages=Missing,Prelude

echo '--- Namespace is not itself a package ---'
run --stripe-packages --focus-packages=Lib

echo '--- Empty selection ---'
run --stripe-packages --focus-packages=

echo '--- Empty name in selection ---'
run --stripe-packages --focus-packages=Target,,Sibling

echo '--- Package mode required ---'
run --focus-packages=Target

echo '--- Upstream skipping requires a focused package ---'
run --stripe-packages --focus-skip-upstream

echo '--- Downstream skipping requires a focused package ---'
run --stripe-packages --focus-skip-downstream

echo '--- Package-directed mode required with either package flag ---'
run --stripe-packages --experimental-package-directed=false --focus-packages=Target
run --sorbet-packages --experimental-package-directed=false --focus-packages=Target

echo '--- LSP requires the whole project ---'
run --stripe-packages --focus-packages=Target --lsp

echo '--- Stored state requires the whole project ---'
run --stripe-packages --focus-packages=Target --store-state=gs,files,names

echo '--- Package generation requires the whole project ---'
run --stripe-packages --focus-packages=Target --gen-packages

echo '--- Suggest-typed requires the whole project ---'
run --stripe-packages --focus-packages=Target --suggest-typed --isolate-error-code=7022 --typed=strict

echo '--- Package declaration errors outside the selection are still reported ---'
# Dereference runfile symlinks so this edit only changes the temporary copy.
cp -RL . "$cache_dir/package-error" || exit 1
awk '{ print } /^class Other < PackageSpec$/ { print "  import Other" }' \
  other/__package.rb > "$cache_dir/package-error/other/__package.rb"
(
  cd "$cache_dir/package-error" || exit 1
  run --stripe-packages --focus-packages=Top::Test --focus-skip-upstream --focus-skip-downstream
)
