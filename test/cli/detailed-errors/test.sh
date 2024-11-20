#!/usr/bin/env bash

set -euo pipefail

main/sorbet --silence-dev-message --max-threads=0 test/cli/detailed-errors 2>&1
