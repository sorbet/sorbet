# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

def boo(&blk)
  begin
    p "in begin"
    yield
  ensure
    p "in ensure"
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#3boo"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: call i64 @sorbet_run_exception_handling{{.*}}
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL{LITERAL}: }

# INITIAL-LABEL: define internal i64 @"func_Object#3boo$block_1"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: call i64 @sorbet_vm_callBlock
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal noundef i64 @"func_Object#3boo$block_1"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT: call i64 @sorbet_vm_callBlock
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT-NOT: call i64 @rb_block_proc
# OPT{LITERAL}: }

boo do
  puts "boohey"
end

