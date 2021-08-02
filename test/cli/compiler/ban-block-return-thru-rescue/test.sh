#!/usr/bin/env bash

set -euo pipefail

outdir="$(mktemp -d)"

cleanup() {
    rm -rf "$outdir"
}
trap cleanup EXIT

go () {
  source="$1"
  sorbet --silence-dev-message --compiled-out-dir="$outdir" --llvm-ir-dir="$outdir" "$source" || echo 'sorbet failed'
}

go supported_case.rb

go block_under_begin.rb
go block_under_rescue.rb
go block_under_else.rb
go block_under_ensure.rb

go begin_under_block.rb
go rescue_under_block.rb
go else_under_block.rb
go ensure_under_block.rb
