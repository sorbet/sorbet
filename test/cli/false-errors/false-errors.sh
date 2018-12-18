#!/bin/bash

main/sorbet --silence-dev-message test/cli/false-errors/false-errors.rb -p error-files 2>&1
