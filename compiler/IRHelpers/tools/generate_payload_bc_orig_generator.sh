#!/bin/bash
set -e

ar=$1
output=$2
afile=$3

$ar -x "$afile" payload.o

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  echo "hi"
elif [[ "$OSTYPE" == "darwin"* ]]; then
  otool  -s __LLVM __bitcode payload.o | tail -n +3 > bitcode.dump
fi

hex=$(head -n 1 bitcode.dump| cut -f 1)
decimal=$(( 16#$hex ))
xxd -r -seek -"$decimal" bitcode.dump "$output"
