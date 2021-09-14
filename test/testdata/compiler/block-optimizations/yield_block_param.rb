# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

def boo(&blk)
  yield
end

# INITIAL-LABEL: "func_Object#3boo"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: call i64 @sorbet_callBlock
# INITIAL{LITERAL}: }

# OPT-LABEL: "func_Object#3boo"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT: call i64 @rb_yield_values_kw
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT{LITERAL}: }

boo do
  puts "boohey"
end

