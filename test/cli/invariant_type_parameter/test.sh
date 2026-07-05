#!/usr/bin/env bash

main/sorbet --silence-dev-message test/cli/invariant_type_parameter/test.rb 2>&1
