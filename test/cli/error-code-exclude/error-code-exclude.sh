#!/bin/bash

main/sorbet --silence-dev-message test/cli/error-code-exclude/error-code-exclude.rb 2>&1

main/sorbet --silence-dev-message test/cli/error-code-exclude/error-code-exclude.rb --error-code-exclude=7002 2>&1

# TODO(jvilk): Remove these once we delete --error-black-list
main/sorbet --silence-dev-message test/cli/error-code-exclude/error-code-exclude.rb --error-black-list=7002 2>&1
