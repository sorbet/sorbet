#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

cwd="$(pwd)"
tmp=$(mktemp -d)
mkdir -p "$tmp/test/cli"
cp -r test/cli/autogen-pkg-autoloader "$tmp/test/cli"

cd "$tmp" || exit 1

mkdir output
dir_to_delete="output/RootPackage/Nested"
inner_dir_to_delete="${dir_to_delete}/Inner"
mkdir -p $inner_dir_to_delete
touch "$inner_dir_to_delete/__file_to_delete.rb"

"$cwd/main/sorbet" --silence-dev-message --stop-after=namer \
  --stripe-packages \
  -p autogen-autoloader:output \
  --autogen-autoloader-modules=RootPackage \
  --autogen-autoloader-exclude-require=byebug \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  test/cli/autogen-pkg-autoloader/{foo,bar,bar2,errors,__package}.rb \
  test/cli/autogen-pkg-autoloader/nested/*.rb \
  test/cli/autogen-pkg-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort | grep -v "_mtime_stamp"); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done

if test -d $dir_to_delete; then
  echo "ERROR: $dir_to_delete exists"
else
  echo "$dir_to_delete correctly deleted"
fi

rm -rf "$tmp"
