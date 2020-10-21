#!/bin/bash

set -euo pipefail

echo '------------------------------------------------------------------------'
main/sorbet --silence-dev-message -p file-table-json --no-stdlib -e ''

echo '------------------------------------------------------------------------'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib foo.rb
rm foo.rb

echo '------------------------------------------------------------------------'
main/sorbet --silence-dev-message -p file-table-json --no-stdlib --dir . --ignore external 2>&1 || true

echo '------------------------------------------------------------------------'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib --dir . --ignore external
rm foo.rb

echo '------------------------------------------------------------------------'
echo 'Note: --dir and -e are meant to be additive'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib --dir . --ignore external -e ''
rm foo.rb

echo '------------------------------------------------------------------------'
