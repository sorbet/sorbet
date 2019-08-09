#!/bin/bash

set -euo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

if [ "$#" -lt 1 ]; then
cat <<EOF
usage:
  $0 <target> [<options>]

example target:
  fuzz_dash_e
  fuzz_doc_symbols
  fuzz_hover

example options:
  --stress-incremental-resolver
EOF
exit 1
fi

target="$1"
shift

echo "building $target"
bazel build "//test/fuzz:$target" --config=fuzz -c opt

echo "making command file"
cmds="$(mktemp)"
trap 'rm "$cmds"' EXIT

# https://stackoverflow.com/questions/2937407
if ! compgen -G "fuzz_crashers/original/crash-*" >/dev/null; then
  echo "fatal: no crashers"
  exit 1
fi

for f in fuzz_crashers/original/crash-*; do
  # this breaks if any of the extra args in $*, or $target, or $f, contain spaces. for instance if this script was
  # called like this: $0 fuzz_dash_e --extra-opt 'foo bar'
  echo "tools/scripts/fuzz_minimize_crash.sh $target $f $* 2>/dev/null" >>"$cmds"
done

echo "running in parallel"
parallel --joblog - <"$cmds"
