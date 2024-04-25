# compiled: true
# frozen_string_literal: true
# typed: true
# run_filecheck: OPT

require_relative './block_call_rb_iterate__2'

# Check that we only see a single call to `rb_iterate` for these two calls to
# `test` with a block. The first use of test shouldn't use rb_iterate because
# the block doesn't use `break` to change control flow, while the second does
# use break and should use `rb_iterate` in the generated code.

# OPT-ENTRY: Init_block_call_rb_iterate__1
# OPT: call i64 @sorbet_rb_iterate(
# OPT-NOT: call i64 @sorbet_rb_iterate(
# OPT{LITERAL}: }

a = test do
  puts "without a break"
end
puts a

b = test do
  puts "with a break"
  break 20
end
puts b
