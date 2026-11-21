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
run --sorbet-packages

echo '--- Prelude selection checks all packages ---'
run --sorbet-packages --focus-package=Prelude

echo '--- Prelude selection checks all packages with downstream skipping ---'
run --sorbet-packages --focus-package=Prelude --focus-extent=none

echo '--- Prelude selection checks all packages with upstream skipping ---'
run --sorbet-packages --focus-package=Prelude --focus-assume-package-dependencies-typecheck

echo '--- Prelude selection checks all packages with both skip flags ---'
run --sorbet-packages --focus-package=Prelude --focus-extent=none --focus-assume-package-dependencies-typecheck

echo '--- Target: includes dependencies and dependents ---'
run --sorbet-packages --focus-package=Target

echo '--- Target: explicit full extent includes dependencies and dependents ---'
run --sorbet-packages --focus-package=Target --focus-extent=full

echo '--- Target: skipping dependents, including explicit test consumers ---'
run --sorbet-packages --focus-package=Target --focus-extent=none

echo '--- Target: skipping inference for dependencies ---'
run --sorbet-packages --focus-package=Target --focus-assume-package-dependencies-typecheck

echo '--- Target: skipping dependents and inference for dependencies ---'
run --sorbet-packages --focus-package=Target --focus-assume-package-dependencies-typecheck --focus-extent=none

# Split every non-prelude stratum into SCCs, then separately combine the entire selection into one stratum.
echo '--- Target: split strata preserve the selection ---'
run --sorbet-packages --focus-package=Target --stratum-split-threshold=1

echo '--- Target: combined strata preserve the selection ---'
run --sorbet-packages --focus-package=Target --min-stratum-size=1000

# A smaller minimum combines only some of the split strata. Selection and inference skipping must stay the same.
echo '--- Target: split and combine strata ---'
run --sorbet-packages --focus-package=Target --stratum-split-threshold=1 --min-stratum-size=5

echo '--- Target: split and combine strata, skipping dependents ---'
run --sorbet-packages --focus-package=Target --stratum-split-threshold=1 --min-stratum-size=5 --focus-extent=none

echo '--- Target: split and combine strata, skipping inference for dependencies ---'
run --sorbet-packages --focus-package=Target --stratum-split-threshold=1 --min-stratum-size=5 --focus-assume-package-dependencies-typecheck

echo '--- Target: split and combine strata with both skip flags ---'
run --sorbet-packages --focus-package=Target --stratum-split-threshold=1 --min-stratum-size=5 \
  --focus-assume-package-dependencies-typecheck --focus-extent=none

echo '--- Multiple names, repeated flag, and duplicate roots ---'
run --sorbet-packages --focus-package=Target,Sibling --focus-package=Target

echo '--- Explicitly selected dependencies are inferred with both skip flags ---'
run --sorbet-packages --focus-package=Target,Lib::Base --focus-assume-package-dependencies-typecheck --focus-extent=none

echo '--- Namespaced package ---'
run --sorbet-packages --focus-package=Lib::Base

echo '--- Cycle: dependencies do not pull in their other consumers ---'
run --sorbet-packages --focus-package=CycleB

echo '--- Cycle: both members are inferred with both skip flags ---'
run --sorbet-packages --focus-package=CycleB --focus-extent=none --focus-assume-package-dependencies-typecheck

echo '--- Test support selection includes explicit test consumers ---'
run --sorbet-packages --focus-package=TestSupport

echo '--- Focused root includes explicit test consumers ---'
run --sorbet-packages --focus-package=Top

echo '--- Explicitly selected tests retain their dependencies ---'
run --sorbet-packages --focus-package=Top::Test

echo '--- A clean focused package succeeds despite errors elsewhere ---'
run --sorbet-packages --focus-package=Top::Test --focus-assume-package-dependencies-typecheck --focus-extent=none

cache_dir=$(mktemp -d)
trap 'rm -rf "$cache_dir"' EXIT

echo '--- Cold cache ---'
run --sorbet-packages --cache-dir="$cache_dir" --focus-package=Target

echo '--- Warm cache, different selection ---'
run --sorbet-packages --cache-dir="$cache_dir" --focus-package=Sibling

echo '--- Warm cache, original selection ---'
run --sorbet-packages --cache-dir="$cache_dir" --focus-package=Target

echo '--- Full check after caching partial checks ---'
run --sorbet-packages --cache-dir="$cache_dir"

echo '--- Unknown package ---'
run --sorbet-packages --focus-package=Target,Missing

echo '--- Unknown package after a prelude ---'
run --sorbet-packages --focus-package=Prelude,Missing

echo '--- Unknown package before a prelude ---'
run --sorbet-packages --focus-package=Missing,Prelude

echo '--- Namespace is not itself a package ---'
run --sorbet-packages --focus-package=Lib

echo '--- Empty selection ---'
run --sorbet-packages --focus-package=

echo '--- Empty name in selection ---'
run --sorbet-packages --focus-package=Target,,Sibling

echo '--- Package mode required ---'
run --focus-package=Target

echo '--- Upstream skipping requires a focused package ---'
run --sorbet-packages --focus-assume-package-dependencies-typecheck

echo '--- Downstream skipping requires a focused package ---'
run --sorbet-packages --focus-extent=none

echo '--- Explicit full extent requires a focused package ---'
run --sorbet-packages --focus-extent=full

echo '--- Invalid focus extent ---'
run --sorbet-packages --focus-package=Target --focus-extent=invalid

echo '--- Package-directed mode required with either package flag ---'
run --sorbet-packages --experimental-package-directed=false --focus-package=Target
run --sorbet-packages --experimental-package-directed=false --focus-package=Target

echo '--- LSP requires the whole project ---'
run --sorbet-packages --focus-package=Target --lsp

echo '--- Stored state requires the whole project ---'
run --sorbet-packages --focus-package=Target --store-state=gs,files,names

echo '--- Package generation requires the whole project ---'
run --sorbet-packages --focus-package=Target --gen-packages

echo '--- Suggest-typed requires the whole project ---'
run --sorbet-packages --focus-package=Target --suggest-typed --isolate-error-code=7022 --typed=strict

echo '--- Package declaration errors outside the selection are still reported ---'
# Dereference runfile symlinks so this edit only changes the temporary copy.
cp -RL . "$cache_dir/package-error" || exit 1
awk '{ print } /^class Other < PackageSpec$/ { print "  import Other" }' \
  other/__package.rb > "$cache_dir/package-error/other/__package.rb"
(
  cd "$cache_dir/package-error" || exit 1
  run --sorbet-packages --focus-package=Top::Test --focus-assume-package-dependencies-typecheck --focus-extent=none
)
