# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

# Verify that we do reify the block when it's invoked in a block.

def boo(array, &blk)
  array.each do |x|
    p x
    yield
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#3boo"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#3boo"
# OPT: call i64 @rb_block_proc
# OPT{LITERAL}: }

# INITIAL-LABEL: define internal i64 @"func_Object#3boo$block_1"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: @sorbet_i_send(%struct.FunctionInlineCache* @ic_call
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

boo([1, 2]) do
  puts "boohey"
end

