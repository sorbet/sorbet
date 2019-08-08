#!/bin/bash

set -exuo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -eq 0 ]; then
cat <<EOF
usage:
  $0 <fuzz_target>

example fuzz_target:
  fuzz_dash_e
  fuzz_doc_symbols
  fuzz_hover
EOF
exit 1
fi

what="$1"
shift

echo "building $what"
bazel build "//test/fuzz:$what" --config=fuzz -c opt

echo "making command file"
cmds="$(mktemp)"
for f in fuzz_crashers/original/crash-*; do
  echo "./tools/scripts/fuzz_minimize_crash.sh '$f' 2>/dev/null" >> "$cmds"
done

echo "running in parallel"
parallel --joblog - < "$cmds"
rm "$cmds"
