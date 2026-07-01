#!/usr/bin/env bash
set -euo pipefail

main/sorbet --silence-dev-message --sorbet-packages test/cli/packager_suggest_nested/ 2>&1
