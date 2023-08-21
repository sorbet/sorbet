#!/usr/bin/env bash

set -euo pipefail

main/sorbet --censor-for-snapshot-tests --silence-dev-message --track-untyped --print=file-table-json --max-threads=0 test/cli/track-untyped 2>&1
