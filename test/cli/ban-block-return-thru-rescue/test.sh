#!/usr/bin/env bash

set -euo pipefail

llvmir="$(mktemp -d)"

cleanup() {
    rm -rf "$llvmir"
}
trap cleanup EXIT

go () {
  source="$1"
  sorbet --silence-dev-message --llvm-ir-folder="$llvmir" "$source" || echo 'sorbet failed'
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
