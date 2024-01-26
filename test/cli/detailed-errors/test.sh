#!/usr/bin/env bash

set -euo pipefail

main/sorbet --silence-dev-message test/cli/detailed-errors 2>&1
