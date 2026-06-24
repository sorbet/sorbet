#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message --uniquely-defined-behavior \
  test/cli/conflicting-definition-multithread/*.rb 2>&1
