#!/usr/bin/env bash

set -euo pipefail

sorbet="$PWD/main/sorbet"

cd "$(dirname "${BASH_SOURCE[0]}")"

# Use tmp file and cat because it seems like the stderr / stdout get
# interleaved poorly otherwise.
tmp_json="$(mktemp)"
# shellcheck disable=SC2064
trap "rm -f '$tmp_json'" EXIT

echo "----- ./*.rb -----------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-table-json:"$tmp_json" ./*.rb 2>&1
cat "$tmp_json"
echo "----- --dir=. ----------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-table-json:"$tmp_json" --dir=. 2>&1
cat "$tmp_json"
echo "----- *.rb -------------------------------------------------------------"
"$sorbet" --silence-dev-message --print file-table-json:"$tmp_json" -- *.rb 2>&1
cat "$tmp_json"
