#!/bin/bash

set -euo pipefail

if ! main/sorbet --silence-dev-message test/cli/error-order/error-order.rb 2>&1 ; then
  echo 'expected to fail!'
  exit 1
fi
