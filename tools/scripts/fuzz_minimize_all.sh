#!/bin/bash

set -exuo pipefail
cd "$(dirname "$0")"
cd "../.."
# we're now at the root of the repo.

bazel build //test/fuzz:fuzz_dash_e --config=fuzz -c opt

cmds="$(mktemp)"

for this_src in fuzz_crashers/original/crash-*; do
    echo "./tools/scripts/fuzz_minimize_crash.sh \"$this_src\" 2>/dev/null" >> "$cmds"
done

parallel --joblog - < "$cmds"
