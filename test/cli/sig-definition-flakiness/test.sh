#!/bin/bash
exec 2>&1

# Will flakily fail if sigs are not defined in a deterministic order.
main/sorbet --silence-dev-message test/cli/sig-definition-flakiness/*.rbi test/cli/sig-definition-flakiness/test.rb

