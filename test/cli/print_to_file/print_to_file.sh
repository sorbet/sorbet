#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after namer --autogen-version=2 \
  -p autogen:autogen.txt -p autogen-msgpack:autogen.msgpack \
  test/cli/print_to_file/{a,b}.rb

echo "--- autogen.txt start ---"
cat autogen.txt
echo "--- autogen.txt end ---"
echo

# Check the byte count in msgpack output
echo "--- autogen.msgpack start ---"
wc -c autogen.msgpack
echo "--- autogen.msgpack end ---"
