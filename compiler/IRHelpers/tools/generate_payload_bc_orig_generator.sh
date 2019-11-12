#!/bin/bash
set -e

ar=$1
output=$2
afile=$3

$ar -x "$afile" payload.o

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  readelf -x .llvmbc payload.o > bitcode.dump
  cat bitcode.dump | tail -n +3 | sed 's/\(.\{13\}\)\(.\{35\}\).*/\2/' > bitcode-xxd.dump
  xxd -r -p < bitcode-xxd.dump > "$output"
elif [[ "$OSTYPE" == "darwin"* ]]; then
  otool  -s __LLVM __bitcode payload.o > bitcode.dump
  cat bitcode.dump | tail -n +3 > bitcode-xxd.dump
  hex=$(head -n 1 bitcode.dump| cut -f 1 -d' ')
  offset=$(( 16#$hex ))
  xxd -r --seek $offset bitcode-xxd.dump "$output"
fi
