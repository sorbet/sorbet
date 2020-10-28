#!/bin/bash

main/sorbet --silence-dev-message test/cli/uninitialized-explanation/uninitialized.rb 2>&1
