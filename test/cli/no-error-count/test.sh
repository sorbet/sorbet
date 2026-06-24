#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message -e '1' --no-error-count 2>&1
main/sorbet --silence-dev-message -e '!' --no-error-count 2>&1
