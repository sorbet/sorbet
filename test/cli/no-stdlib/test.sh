#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message -e "puts 'Hello > World'" 2>&1

echo ------------------------------

main/sorbet --silence-dev-message --no-stdlib -e "puts 'Hello > World'" 2>&1
