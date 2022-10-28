#!/bin/bash
set -eu

preamble="# frozen_string_literal: true
# typed: true
"

rm -rf output
mkdir -p output

main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader:output \
  --autogen-autoloader-modules={Foo,Yabba} \
  --autogen-autoloader-exclude-require=byebug \
  --autogen-autoloader-ignore=scripts/ \
  --autogen-autoloader-preamble "$preamble" \
  test/cli/autogen-autoloader/{example1,example2,example3,errors}.rb \
  test/cli/autogen-autoloader/scripts/baz.rb 2>&1

for file in $(find output -type f | sort | grep -v "_mtime_stamp"); do
  printf "\n--- %s\n" "$file"
  cat "$file"
done


echo
echo "--- missing output directory"
set +e
main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader \
  test/cli/autogen-autoloader/scripts/baz.rb 2>&1
set -e


echo
echo "--- in-place writes"
rm -rf inplace-output
mkdir -p inplace-output
touch inplace-output/delete_me.rb # Expect this to be deleted from the output

main/sorbet --silence-dev-message --stop-after=namer \
  -p autogen-autoloader:inplace-output \
  --autogen-autoloader-modules=Foo \
  test/cli/autogen-autoloader/inplace.rb

find inplace-output | sort | grep -v "_mtime_stamp"

echo
echo "--- strip-prefixes and root rename"
rm -rf strip-output
mkdir -p strip-output
main/sorbet --silence-dev-message --stop-after=namer \
  -p autogen-autoloader:strip-output \
  --autogen-autoloader-modules=Foo \
  --autogen-autoloader-root my-autoloader/ \
  --autogen-autoloader-strip-prefix test/cli/ \
  --autogen-registry-module "Primus::Require" \
  test/cli/autogen-autoloader/inplace.rb

cat strip-output/Foo.rb

echo
echo "--- with different root object"
rm -rf root-object
mkdir -p root-object
main/sorbet --silence-dev-message --stop-after=namer -p autogen-autoloader:root-object \
  --autogen-autoloader-modules=Foo \
  --autogen-root-object=MyRootObject \
  test/cli/autogen-autoloader/inplace.rb 2>&1

cat root-object/Foo.rb
