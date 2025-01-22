#!/bin/bash
set -eu

echo "Version 6:"
main/sorbet --silence-dev-message --stop-after namer --autogen-version=6 \
  -p autogen:autogen6.txt -p autogen-msgpack:autogen6.msgpack --autogen-behavior-allowed-in-rbi-files-paths="test/cli/autogen_print_rbi/behavior_allowed.rbi" \
  test/cli/autogen_print_rbi/*.{rb,rbi}

cat autogen6.txt

echo
shasum autogen6.msgpack
