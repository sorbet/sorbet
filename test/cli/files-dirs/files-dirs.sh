#!/bin/bash

set -euo pipefail

echo '------------------------------------------------------------------------'
main/sorbet --silence-dev-message -p file-table-json --no-stdlib -e ''

echo '------------------------------------------------------------------------'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib foo.rb
rm foo.rb

echo '------------------------------------------------------------------------'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib --dir .
rm foo.rb

echo '------------------------------------------------------------------------'
echo 'Note: --dir and -e are meant to be additive'
touch foo.rb
main/sorbet --silence-dev-message -p file-table-json --no-stdlib --dir . -e ''
rm foo.rb

echo '------------------------------------------------------------------------'
