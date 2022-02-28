#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message test/cli/expected-got/expected-got.rb 2>&1
