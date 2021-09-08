# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

def foo
  yield
end

def bar(&blk)
  foo(&blk)
end

# INITIAL-LABEL: "func_Object#3bar"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: @sorbet_getPassedBlockHandler
# INITIAL: call i64 @sorbet_callFuncWithCache
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

# OPT-LABEL: "func_Object#3bar"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT: @sorbet_getPassedBlockHandler
# OPT: call i64 @sorbet_callFuncWithCache
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT{LITERAL}: }

bar do
  puts "bar"
end
