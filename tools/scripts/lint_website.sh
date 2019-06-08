#!/bin/bash

set -euo pipefail

cd "$(dirname "$0")/../../website"

yarn
if ! yarn prettier-check; then
  echo
  echo "^ To fix these files, run 'yarn prettier' in the website/ folder locally."
  exit 1
fi
