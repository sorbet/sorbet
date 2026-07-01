#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message --stop-after namer \
  -p autogen \
  test/cli/autogen_print_alias_scopes/*.rb
