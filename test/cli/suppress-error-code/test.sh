#!/bin/bash

main/sorbet --silence-dev-message test/cli/suppress-error-code/suppress-error-code.rb 2>&1

main/sorbet --silence-dev-message test/cli/suppress-error-code/suppress-error-code.rb --suppress-error-code=7002 2>&1

# TODO(jvilk): Remove these once we delete --error-black-list
main/sorbet --silence-dev-message test/cli/suppress-error-code/suppress-error-code.rb --error-black-list=7002 2>&1
