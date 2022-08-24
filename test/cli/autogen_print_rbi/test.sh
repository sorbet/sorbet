#!/bin/bash
set -eu

echo "Version 3:"
main/sorbet --silence-dev-message --stop-after namer --autogen-version=3 \
  -p autogen:autogen3.txt -p autogen-msgpack:autogen3.msgpack \
  test/cli/autogen_print_rbi/*.{rb,rbi}

cat autogen3.txt

echo
echo
echo "Version 4:"
main/sorbet --silence-dev-message --stop-after namer --autogen-version=4 \
  -p autogen:autogen4.txt -p autogen-msgpack:autogen4.msgpack --autogen-behavior-allowed-in-rbi-files-paths="test/cli/autogen_print_rbi/behavior_allowed.rbi" \
  test/cli/autogen_print_rbi/*.{rb,rbi}

cat autogen4.txt

echo
shasum autogen3.msgpack autogen4.msgpack

echo
diff autogen3.txt autogen4.txt
