#!/bin/bash

set -euo pipefail

base="$( cd "$(dirname "$0")" ; pwd -P )"/..

rb=$1

# ensure that the extension is built
"$base/test/run_sorbet.sh" "$rb"

llvmir="$PWD/llvmir"

pushd "$base" > /dev/null
tools/scripts/lldb.sh "$llvmir" "$rb"
popd > /dev/null
