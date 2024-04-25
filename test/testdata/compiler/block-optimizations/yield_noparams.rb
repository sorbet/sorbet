# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

def foo
  yield
end

# INITIAL-LABEL: define internal i64 @"func_Object#3foo"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: call i64 @sorbet_vm_callBlock
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#3foo"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT: call i64 @sorbet_vm_callBlock
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT{LITERAL}: }

foo do
  puts "heey"
end

