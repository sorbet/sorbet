#!/bin/bash
set -eu

echo "--- autogen-subclasses-ignore-absolute ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-subclasses \
  --autogen-subclasses-parent=Opus::Parent \
  --autogen-subclasses-ignore=/test \
  test/cli/autogen-subclasses-ignore/ignored/ignored.rb \
  test/cli/autogen-subclasses-ignore/not-ignored/not-ignored.rb

echo "--- autogen-subclasses-ignore-relative ---"
main/sorbet --silence-dev-message --stop-after=namer -p autogen-subclasses \
  --autogen-subclasses-parent=Opus::Parent \
  --autogen-subclasses-ignore=ignored \
  test/cli/autogen-subclasses-ignore/ignored/ignored.rb \
  test/cli/autogen-subclasses-ignore/not-ignored/not-ignored.rb
