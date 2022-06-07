#!/bin/bash
set -u

main/sorbet --silence-dev-message --stripe-mode \
  test/cli/conflicting-definition-multithread/*.rb 2>&1
