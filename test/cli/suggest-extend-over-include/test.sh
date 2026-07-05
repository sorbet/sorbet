#!/bin/bash
main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/suggest-extend-over-include/suggest-extend-over-include.rb 2>&1
