#!/usr/bin/env bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/dedent_hint/dedent_hint.rb 2>&1
