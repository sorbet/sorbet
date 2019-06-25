#!/bin/bash
set -eu

echo "--- autogen-subclasses ---"
main/sorbet --silence-dev-message --stop-after=resolver -p autogen-subclasses --autogen-subclasses-parent=Opus::Mixin --autogen-subclasses-parent=Opus::Parent test/cli/autogen-subclasses/a.rb
