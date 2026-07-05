#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/exception-loop/test.rb 2>&1
