#!/bin/bash

main/sorbet --silence-dev-message test/cli/isolate-error-code/isolate-error-code.rb --isolate-error-code=7017 2>&1

# TODO(jvilk): Remove when we remove --error-white-list
main/sorbet --silence-dev-message test/cli/isolate-error-code/isolate-error-code.rb --error-white-list=7017 2>&1
