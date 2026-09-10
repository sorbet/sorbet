#!/usr/bin/env bash

# Characterize legacy Test:: constant lookup in monolithic and package-directed
# modes. In particular, package-directed production strata run before legacy
# test sources have defined their constants.

cd test/cli/legacy-test-constant-resolution || exit 1

args=(--silence-dev-message --censor-for-snapshot-tests --sorbet-packages --max-threads=0)

echo '------ monolithic -------------------------------'
../../../main/sorbet "${args[@]}" . 2>&1

args+=(--experimental-package-directed)

echo '------ package-directed: isolated production, no import ------'
../../../main/sorbet "${args[@]}" target prod_none 2>&1

echo '------ package-directed: isolated production, normal import ------'
../../../main/sorbet "${args[@]}" target prod_normal 2>&1

echo '------ package-directed: isolated production, test import ------'
../../../main/sorbet "${args[@]}" target prod_test 2>&1

echo '------ package-directed: isolated test unit, no import ------'
../../../main/sorbet "${args[@]}" target unit_none 2>&1

echo '------ package-directed: unimported target test SCC is later ------'
../../../main/sorbet "${args[@]}" late_target unit_late 2>&1

echo '------ package-directed: unimported target test SCC is earlier ------'
../../../main/sorbet "${args[@]}" early_target early_bridge unit_early 2>&1

echo '------ package-directed: isolated test unit, test import ------'
../../../main/sorbet "${args[@]}" target unit_import 2>&1

echo '------ package-directed: isolated helper, helper import ------'
../../../main/sorbet "${args[@]}" target helper_import 2>&1

echo '------ package-directed: isolated helper, test-unit import ------'
../../../main/sorbet "${args[@]}" target helper_unit_import 2>&1

# Running the complete graph lets unrelated test_import edges load Target's
# test SCC before some production strata. This characterizes the corresponding
# traversal-order-sensitive behavior.
echo '------ package-directed: complete graph ----------------------'
../../../main/sorbet "${args[@]}" . 2>&1
