#!/bin/bash

set -e

main/sorbet --silence-dev-message --report-ambiguous-defs test/cli/ambiguous-definitions/foo.rb 2>&1

