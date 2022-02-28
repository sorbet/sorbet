#!/bin/bash

echo ----- Abbreviated help output: -------------------------------------------
echo

main/sorbet 2>&1

echo
echo ----- Abbreviated help output with empty config file: --------------------
echo

mkdir -p sorbet
touch sorbet/config
trap 'rm -rf sorbet/' EXIT

main/sorbet 2>&1

echo
echo ----- Full help output: --------------------------------------------------
echo

# capturing the full help output is really brittle, because it needlessly
# requires changing after every new option is added or deleted, and we don't
# need to be in the business of testing our arg parsing library's ability to
# generate help text.
#
# Instead just show that `--help` outputs more `... options:` sections than
# would normally be printed with just `-h`, but none of the options in those
# sections.

main/sorbet --help 2>&1 | \
  grep 'options:'

echo --------------------------------------------------------------------------
