#!/usr/bin/env bash

root="$PWD"

cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1

"$root/main/sorbet" --silence-dev-message --sorbet-packages . 2>&1
