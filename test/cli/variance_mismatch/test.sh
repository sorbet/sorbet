#!/usr/bin/env bash

main/sorbet --silence-dev-message test/cli/variance_mismatch/test.rb 2>&1
