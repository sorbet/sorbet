#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message test/cli/empty-file/empty.rb 2>&1
