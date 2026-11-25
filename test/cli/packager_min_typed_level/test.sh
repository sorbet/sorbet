#!/usr/bin/env bash

set -euo pipefail

cd test/cli/packager_min_typed_level || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
