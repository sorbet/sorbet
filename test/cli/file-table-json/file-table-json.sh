#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message --no-stdlib -p file-table-json test/cli/file-table-json/file-table-json.rb
