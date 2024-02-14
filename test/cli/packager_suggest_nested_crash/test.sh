#!/usr/bin/env bash

cd test/cli/packager_suggest_nested_crash || exit 0
../../../main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1
