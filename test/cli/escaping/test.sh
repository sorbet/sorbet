#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message test/cli/escaping/escaping.rb 2>&1
