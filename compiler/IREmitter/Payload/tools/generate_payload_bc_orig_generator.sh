#!/bin/bash

set -euo pipefail
ar="$(pwd)/$1"
link="$(pwd)/$2"
output="$(pwd)/$3"
xxd="$(pwd)/$4"
afile="$(pwd)/$5"
workdir=$(mktemp -d)

cd "$workdir"

extract_bitcode() {
  local object_file="$1"
  local bc_file="$object_file.bc"
  rm -rf "$object_file"
  $ar -x "$afile" "$object_file"

  if [[ "$OSTYPE" == "linux-gnu" ]]; then
    readelf -x .llvmbc "$object_file" > bitcode.dump
    tail -n +3 < bitcode.dump | sed 's/\(.\{13\}\)\(.\{35\}\).*/\2/' > bitcode-xxd.dump
    $xxd -r -p < bitcode-xxd.dump > "$bc_file"
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    otool  -s __LLVM __bitcode "$object_file" > bitcode.dump
    tail -n +3 < bitcode.dump > bitcode-xxd.dump
    hex=$(head -n 1 bitcode-xxd.dump | cut -f 1)
    offset=$(( 16#$hex ))
    $xxd -r --seek -$offset bitcode-xxd.dump "$bc_file"
  fi
}

# Extract bitcode from all objects from the archive
$ar -t "$afile" | grep '.o$' | while read -r ofile; do
  extract_bitcode "$ofile"
done

$link -o "$output" ./*.bc

rm -rf "$workdir"
