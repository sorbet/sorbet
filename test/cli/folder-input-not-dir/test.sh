#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message --dir test/cli/folder-input-not-dir/foo.rb 2>&1
