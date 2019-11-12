#!/bin/bash
set -e

ar=$1
output=$2
afile=$3

$ar -x "$afile" payload.o

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  readelf -x .llvmbc payload.o > bitcode.dump
  tail -n +3 < bitcode.dump | sed 's/\(.\{13\}\)\(.\{35\}\).*/\2/' > bitcode-xxd.dump
  xxd -r -p < bitcode-xxd.dump > "$output"
elif [[ "$OSTYPE" == "darwin"* ]]; then
  otool  -s __LLVM __bitcode payload.o > bitcode.dump
  tail -n +3 < bitcode.dump > bitcode-xxd.dump
  hex=$(head -n 1 bitcode-xxd.dump | cut -f 1)
  offset=$(( 16#$hex ))
  xxd -r --seek -$offset bitcode-xxd.dump "$output"
fi
