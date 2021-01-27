#!/bin/bash

main/sorbet --silence-dev-message test/cli/error-code-include/error-code-include.rb --error-code-include=7017 2>&1

# TODO(jvilk): Remove when we remove --error-white-list
main/sorbet --silence-dev-message test/cli/error-code-include/error-code-include.rb --error-white-list=7017 2>&1
