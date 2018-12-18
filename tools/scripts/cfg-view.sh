#!/bin/bash
set -e

dot=$(mktemp)
svg=$(mktemp).svg

dir="$( dirname "${BASH_SOURCE[0]}" )"
"$dir"/../../bazel-bin/main/sorbet --silence-dev-message --suppress-non-critical --print cfg "$@" > "$dot"
dot -Tsvg "$dot" > "$svg"
open -a "Google Chrome" "$svg"
