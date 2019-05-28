#!/bin/bash
set -eu

echo "--- autogen-classlist ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-classlist test/cli/autogen-classlist/a.rb

echo "--- invalid: class in method ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-classlist test/cli/autogen-classlist/b.rb 2>&1
