#!/bin/bash

set -euo pipefail

main/sorbet --silence-dev-message --stop-after=namer -p autogen test/cli/autogen-ignore/autogen-ignore.rb 2>&1
