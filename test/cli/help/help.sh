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

main/sorbet --help 2>&1 | \
  # max-threads help output (default) is hardware-specific
  grep -v -- '--max-threads' | \
  # --suggest-sig only shows up in debug builds
  grep -v -- '--suggest-sig' | \
  # --suggest-sig output spills out onto two lines
  grep -v -- 'debug builds'

echo --------------------------------------------------------------------------
