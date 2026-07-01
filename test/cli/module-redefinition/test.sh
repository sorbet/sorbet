#!/bin/bash

set -euo pipefail
main/sorbet --censor-for-snapshot-tests --silence-dev-message test/cli/module-redefinition/module-redefinition-*.rb 2>&1
