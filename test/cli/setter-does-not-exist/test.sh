#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/setter-does-not-exist/test.rb 2>&1
