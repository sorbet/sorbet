#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after=namer \
  test/cli/conflicting-definition/{a,b}.rb 2>&1
