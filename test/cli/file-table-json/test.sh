#!/bin/bash

set -euo pipefail

echo "----- -p file-table-json ------------------------------------------------"
main/sorbet --silence-dev-message -p file-table-json test/cli/file-table-json/file-table-json.rb
echo "----- --no-stdlib -p file-table-json -----------------------------------"
main/sorbet --silence-dev-message --no-stdlib -p file-table-json test/cli/file-table-json/file-table-json.rb
echo "----- -p file-table-full-json ------------------------------------------"
main/sorbet --silence-dev-message -p file-table-full-json test/cli/file-table-json/file-table-json.rb
echo "----- --no-stdlib -p file-table-full-json ------------------------------"
main/sorbet --silence-dev-message --no-stdlib -p file-table-full-json test/cli/file-table-json/file-table-json.rb
