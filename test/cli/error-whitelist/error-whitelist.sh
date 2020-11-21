#!/bin/bash
main/sorbet --silence-dev-message test/cli/error-whitelist/error-whitelist.rb --error-white-list=7017 2>&1
main/sorbet --silence-dev-message test/cli/error-whitelist/error-whitelist.rb --isolate-error-code=7017 2>&1
