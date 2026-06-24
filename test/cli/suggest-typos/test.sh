#!/bin/bash

set -euo pipefail
main/sorbet --censor-for-snapshot-tests --silence-dev-message -e '1.to_' 2>&1
