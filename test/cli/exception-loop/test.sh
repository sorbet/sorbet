#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message test/cli/exception-loop/test.rb 2>&1
