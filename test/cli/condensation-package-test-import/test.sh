#!/usr/bin/env bash

# This test exercises dependencies between test packages in the condensation
# graph. The test package for A must be checked after the test package for B,
# with C existing to push B's test package later in the traversal. If the import
# dependency is not modeled correctly, A's test package can be checked first,
# causing a resolution error for the superclass in A's test.

cd test/cli/condensation-package-test-import || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --experimental-package-directed \
  --max-threads=0 a b c 2>&1
