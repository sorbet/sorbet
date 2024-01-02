#!/usr/bin/env bash

set -euo pipefail

main/sorbet --silence-dev-message test/cli/multiple-return-loc 2>&1
