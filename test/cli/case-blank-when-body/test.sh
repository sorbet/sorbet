#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/case-blank-when-body/blank.rb 2>&1

