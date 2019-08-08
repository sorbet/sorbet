#!/bin/bash

set -euo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 1 ]; then
cat <<EOF
usage:
  $0 <fuzz_target> [<options>]

example fuzz_target:
  fuzz_dash_e
  fuzz_doc_symbols
  fuzz_hover

example options:
  --stress-incremental-resolver
EOF
exit 1
fi

fuzz_target="$1"
shift

echo "building $fuzz_target"
bazel build "//test/fuzz:$fuzz_target" --config=fuzz -c opt

echo "making command file"
cmds="$(mktemp)"
for f in fuzz_crashers/original/crash-*; do
  # this breaks if any of the extra args in $@ have spaces, for instance if this script was called like this:
  # $0 fuzz_dash_e --extra-opt 'foo bar'
  echo "tools/scripts/fuzz_minimize_crash.sh $f $* 2>/dev/null" >>"$cmds"
done

echo "running in parallel"
parallel --joblog - <"$cmds"
rm "$cmds"
