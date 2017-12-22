#!/bin/bash
set -eux

# re-record with tools/scripts/update_exp_files.sh

diff -u test/end-to-end-test-output <(main/ruby-typer test/end-to-end-test-input.rb 2>&1 | sed -r -e 's,(rbi/stdlib.rbi:)[0-9]+,\1__LINE__,')
