#!/usr/bin/env bash

main/sorbet --silence-dev-message --stripe-packages test/cli/packager_suggest_nested/ 2>&1
