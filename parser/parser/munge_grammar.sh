#!/usr/bin/env bash

infile="$1"
outfile="$2"
replacement="$(basename ${outfile} .ypp)"

sed -e "s/TYPEDRUBY/${replacement}/g" "$infile" > "$outfile"

# Try to make sure we made some kind of modification.
if cmp -s "$infile" "$outfile"; then
    echo "didn't substitute TYPEDRUBY with ${replacement}"
    exit 1
fi

exit 0
