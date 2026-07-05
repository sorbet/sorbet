#!/usr/bin/env bash

# This test exercises test helper dependencies in the condensation graph,
# ensuring that we model those dependencies correctly. In the example, the test
# package for A must be checked after the test package for B, with C existing to
# push the B test package later in the condensation graph traversal. If the
# test_import dependency isn't correctly modeled in the condensation graph, A's
# test package might be checked before B's, at which point we'll see a
# resolution error for the superclass in A's test.

cd test/cli/condensation-package-test-import || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --experimental-package-directed \
  --max-threads=0 a b c 2>&1
