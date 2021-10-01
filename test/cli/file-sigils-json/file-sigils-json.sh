#!/usr/bin/env bash

set -euo pipefail

sorbet="$PWD/main/sorbet"

cd "$(dirname "${BASH_SOURCE[0]}")"

echo "----- ./*.rb -----------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-sigils-json ./*.rb 2>&1
echo "----- --dir=. ----------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-sigils-json --dir=. 2>&1
echo "----- *.rb -------------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-sigils-json -- *.rb 2>&1
