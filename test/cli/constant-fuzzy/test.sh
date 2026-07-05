#!/bin/bash

main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/constant-fuzzy/constant-fuzzy.rb 2>&1
