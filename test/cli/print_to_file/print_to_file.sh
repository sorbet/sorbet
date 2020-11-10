#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after namer --stripe-packages \
  -p autogen:autogen.txt \
  test/cli/print_to_file/*.rb

echo "--- autogen.txt start ---"
cat autogen.txt
echo "--- autogen.txt end ---"
echo

shasum autogen.msgpack

echo
# Do not allow a print option to use inconsistent paths
main/sorbet --silence-dev-message -p cfg -p cfg:out -e '1' 2>&1
