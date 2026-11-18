#!/bin/bash

cwd="$(pwd)"

echo "###### Typecheck ######"

args=(
  "--max-threads=0"
  "--silence-dev-message"
  "--sorbet-packages"
  "--packager-layers=base_lib,app_service,test_infra,test_helper,test_unit"
  "."
)

cd test/cli/packager-layers-underscored || exit 1
"$cwd/main/sorbet" "${args[@]}" 2>&1

tmp="$(mktemp -d)"
for file in $(find . -name '*.rb' | sort); do
  mkdir -p "$tmp/$(dirname "$file")"
  cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

echo
echo "###### Running gen-packages ######"

"$cwd/main/sorbet" "${args[@]}" --gen-packages -a 2>&1

for file in $(find . -name '__package.rb' | LC_COLLATE=C sort); do
  echo
  echo "###### cat $file ######"
  cat "$file"
done
