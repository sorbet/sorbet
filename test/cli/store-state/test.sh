#!/bin/bash
set -e
main/sorbet --silence-dev-message -e 'class Foo; end' --store-state symtab1,names1,files1
if [ ! -f symtab1 ]; then
  echo "symbol table wasn't created"
  exit 1
fi
if [ ! -f names1 ]; then
  echo "name table wasn't created"
  exit 1
fi
if [ ! -f files1 ]; then
  echo "file table wasn't created"
  exit 1
fi
main/sorbet --silence-dev-message -e 'class Foo; end' --store-state symtab2,names2,files2
diff symtab1 symtab2 # there should be no difference
diff names1 names2 # there should be no difference
diff files1 files2 # there should be no difference
