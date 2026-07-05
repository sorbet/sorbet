#!/bin/bash
script="$1"
expect="$2"

"$script" > "$expect" 2>/dev/null || :
