#!/bin/bash

# --store-state builds Sorbet's payload, which isn't packaged
main/sorbet --silence-dev-message --sorbet-packages -e 'class Foo; end' --store-state symtab,names,files 2>&1
echo "exit code: $?"
if [ -f symtab ]; then
  echo "symbol table was created"
fi
