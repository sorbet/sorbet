#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after namer --autogen-version=2 \
  -p autogen:autogen.txt -p autogen-msgpack:autogen.msgpack \
  test/cli/print_to_file/{a,b}.rb

echo "--- autogen.txt start ---"
cat autogen.txt
echo "--- autogen.txt end ---"
echo

shasum autogen.msgpack
