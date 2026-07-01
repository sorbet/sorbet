#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message -e '!' --suppress-non-critical 2>&1
echo $?
