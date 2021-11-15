# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL OPT

def baz(&blk)
  blk.call("baz")
end

# INITIAL-LABEL: define internal i64 @"func_Object#3baz"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: call i64 @sorbet_vm_callBlock
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 "func_Object#3baz"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT: call i64 @sorbet_vm_callBlock
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT{LITERAL}: }

baz do |s|
  p s
end
