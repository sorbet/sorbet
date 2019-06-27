#!/bin/bash
set -eu

echo "--- autogen-subclasses-ignore ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-subclasses \
  --autogen-subclasses-parent=Opus::Mixin \
  --autogen-subclasses-parent=Opus::Parent \
  --autogen-subclasses-parent=Opus::SafeMachine \
  --autogen-subclasses-parent=Chalk::ODM::Model \
  --autogen-subclasses-ignore=test \
  test/cli/autogen-subclasses-ignore/a.rb
