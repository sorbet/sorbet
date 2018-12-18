#!/bin/bash

main/sorbet --silence-dev-message --configatron-file=test/cli/configatron/configatron.yaml test/cli/configatron/configatron.rb 2>&1
