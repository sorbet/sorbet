# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

def foo
  yield
end

# INITIAL-LABEL: "func_Object#foo"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: call i64 @sorbet_callBlock

# OPT-LABEL: "func_Object#foo"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT: call i64 @rb_yield_values_kw
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc

def boo(&blk)
  yield
end

# INITIAL-LABEL: "func_Object#boo"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: call i64 @sorbet_callBlock

# OPT-LABEL: "func_Object#boo"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT: call i64 @rb_yield_values_kw
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc

def bar(&blk)
  foo(&blk)
end

# INITIAL-LABEL: "func_Object#bar"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: @sorbet_getPassedBlockHandler
# INITIAL: call i64 @sorbet_callFuncWithCache

# OPT-LABEL: "func_Object#bar"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT: @sorbet_getPassedBlockHandler
# OPT: call i64 @sorbet_callFuncWithCache

def baz(&blk)
  blk.call("baz")
end

# INITIAL-LABEL: "func_Object#baz"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL: call i64 @sorbet_callBlock
# INITIAL{LITERAL}: }

# OPT-LABEL: "func_Object#baz"
# OPT-NOT: call i64 @sorbet_getMethodBlockAsProc
# OPT: call i64 @rb_yield_values_kw
# OPT{LITERAL}: }

foo do
  puts "heey"
end

boo do
  puts "boohey"
end

bar do
  puts "bar"
end

baz do
end
