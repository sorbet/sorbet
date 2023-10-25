#!/bin/bash
set -eu

echo "Version 5:"
main/sorbet --silence-dev-message --stop-after namer --autogen-version=5 \
  -p autogen:autogen5.txt -p autogen-msgpack:autogen5.msgpack --autogen-behavior-allowed-in-rbi-files-paths="test/cli/autogen_print_rbi/behavior_allowed.rbi" \
  test/cli/autogen_print_rbi/*.{rb,rbi}

cat autogen5.txt

echo
shasum autogen5.msgpack
