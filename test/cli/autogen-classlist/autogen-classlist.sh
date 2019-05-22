#!/bin/bash
set -eu

main/sorbet --silence-dev-message --stop-after=namer -p autogen-classlist test/cli/autogen-classlist/a.rb
