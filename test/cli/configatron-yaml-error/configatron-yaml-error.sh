#!/bin/bash

main/sorbet --silence-dev-message --configatron-file=test/cli/configatron-yaml-error/configatron-yaml-error.yaml test/cli/configatron-yaml-error/configatron-yaml-error.rb 2>&1
