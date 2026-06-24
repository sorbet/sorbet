#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/no-error-sections/*.rb --no-error-sections 2>&1
