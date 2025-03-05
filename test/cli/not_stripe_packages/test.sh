#!/usr/bin/env bash

set -euo pipefail

# We currently populate core::File::Flags::isPackage only via the filename,
# regardless of whether the `--stripe-packages` flag has been passed.
#
# This makes it easy to have behavior that leaks `--stripe-packages`-specific
# behavior into the pipeline. Long term, we should figure out how to fix this.
if main/sorbet --silence-dev-message test/cli/not_stripe_packages/__package.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
