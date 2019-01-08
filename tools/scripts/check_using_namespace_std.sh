#!/bin/bash

set -euo pipefail

red=$'\x1b[0;31m'
cnone=$'\x1b[0m'

if git grep --line '^\s*using namespace std;' '*.h'; then
  echo
  echo "${red}[ERR!]${cnone} Please don't use 'using namespace std;' in header files."
  echo "Header files should fully-qualify standard library names, but feel free"
  echo "to use 'using namespace std;' in *.cc files."
  echo
  echo "The offending locations are listed above."
  echo

  exit 1
fi
