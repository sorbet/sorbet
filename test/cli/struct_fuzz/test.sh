#!/usr/bin/env bash

main/sorbet --censor-for-snapshot-tests --silence-dev-message --no-stdlib test/cli/struct_fuzz/struct_fuzz.rb 2>&1
