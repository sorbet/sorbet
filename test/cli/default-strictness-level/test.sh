#!/usr/bin/env bash

main/sorbet --silence-dev-message --default-strictness-level=true test/cli/default-strictness-level/default-strictness-level.rb 2>&1
