#!/bin/bash
set -euo pipefail

main/sorbet --silence-dev-message --dir test/cli/folder-input-not-found/files 2>&1
