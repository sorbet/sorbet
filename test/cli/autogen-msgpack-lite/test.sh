#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after namer --autogen-version=6 \
  -p autogen:autogen.txt -p autogen-msgpack:autogen.msgpack \
  --autogen-msgpack-skip-reference-metadata \
  --remove-path-prefix=test/cli/ \
  test/cli/autogen-msgpack-lite \
  test/cli/autogen-msgpack-lite/file_with_no_dot

echo "--- autogen.txt start ---"
cat autogen.txt
echo "--- autogen.txt end ---"
echo

shasum autogen.msgpack

