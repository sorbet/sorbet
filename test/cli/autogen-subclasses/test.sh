#!/bin/bash
set -eu

echo "--- autogen-subclasses ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-subclasses \
  --autogen-subclasses-parent=Opus::Mixin \
  --autogen-subclasses-parent=Opus::Parent \
  --autogen-subclasses-parent=Opus::IDontExist \
  --autogen-subclasses-parent=Opus::NeverSubclassed \
  --autogen-subclasses-parent=MyMixin \
  --autogen-subclasses-parent=Bopus::Parent \
  test/cli/autogen-subclasses/
