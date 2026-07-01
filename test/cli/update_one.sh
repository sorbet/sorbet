#!/bin/bash

set -euo pipefail
script="$1"
expect="$2"

"$script" > "$expect" 2>/dev/null || :
