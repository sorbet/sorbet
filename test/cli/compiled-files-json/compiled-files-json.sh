#!/usr/bin/env bash

set -euo pipefail

sorbet="$PWD/main/sorbet"

cd "$(dirname "${BASH_SOURCE[0]}")"

"$sorbet" --silence-dev-message --print compiled-files-json ./*.rb 2>&1
