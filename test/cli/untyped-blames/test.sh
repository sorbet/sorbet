#!/usr/bin/env bash

set -euo pipefail

main/sorbet --silence-dev-message --track-untyped --print=untyped-blame:untyped-blames.json --max-threads=0 test/cli/untyped-blames 2>&1
cat untyped-blames.json | jq 'sort_by(.name)'
