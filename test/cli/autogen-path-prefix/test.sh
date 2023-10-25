#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after namer --autogen-version=5 \
  -p autogen:autogen.txt -p autogen-msgpack:autogen.msgpack \
  --remove-path-prefix=test/cli/ \
  test/cli/autogen-path-prefix \
  test/cli/autogen-path-prefix/file_with_no_dot

echo "--- autogen.txt start ---"
cat autogen.txt
echo "--- autogen.txt end ---"
echo

shasum autogen.msgpack

