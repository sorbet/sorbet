#!/usr/bin/env bash

set -euo pipefail

cd test/cli/packager_min_typed_level_test_packages || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --experimental-test-packages --max-threads=0 . 2>&1
