#!/bin/bash
set -eu

echo "--- autogen-subclasses ---"
main/sorbet --silence-dev-message --stop-after=resolver -p autogen-subclasses test/cli/autogen-subclasses/a.rb
