#!/bin/bash

set -e

main/sorbet --silence-dev-message test/cli/ambiguous-definitions/foo.rb 2>&1

