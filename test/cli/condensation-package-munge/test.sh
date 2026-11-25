#!/usr/bin/env bash

# This test exercises the interaction of package-directed mode, test-packages, and
# packages with files from munged paths.

cd test/cli/condensation-package-munge || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages \
  --extra-package-files-directory-prefix-slash=munged/ \
  --experimental-package-directed \
  --max-threads=0 a b munged 2>&1
