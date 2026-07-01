#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message --suppress-non-critical test/cli/make_accessible/suit.rb 2>&1
