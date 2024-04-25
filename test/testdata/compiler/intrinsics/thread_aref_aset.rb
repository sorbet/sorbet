# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL OPT

extend T::Sig

sig {returns(Thread)}
def thread_current
  Thread.current
end

# INITIAL-LABEL: define internal i64 @"func_Object#14thread_current
# INITIAL-NOT: call i64 @sorbet_i_send
# INITIAL: call i64 @sorbet_Thread_current
# INITIAL-NOT: call i64 @sorbet_i_send
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#14thread_current
# OPT-NOT: call i64 @callFuncWithCache
# OPT: call i64 @rb_thread_current
# OPT-NOT: call i64 @callFuncWithCache
# OPT{LITERAL}: }

sig {returns(Thread)}
def thread_main
  Thread.main
end

# INITIAL-LABEL: define internal i64 @"func_Object#11thread_main
# INITIAL-NOT: call i64 @sorbet_i_send
# INITIAL: call i64 @sorbet_Thread_main
# INITIAL-NOT: call i64 @sorbet_i_send
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#11thread_main
# OPT-NOT: call i64 @callFuncWithCache
# OPT: call i64 @rb_thread_main
# OPT-NOT: call i64 @callFuncWithCache
# OPT{LITERAL}: }

sig {params(thread: Thread).returns(T.untyped)}
def thread_aref_constant(thread)
  thread[:my_key]
end

# INITIAL-LABEL: define internal i64 @"func_Object#20thread_aref_constant
# INITIAL: call i64 @sorbet_Thread_square_br_symarg
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#20thread_aref_constant
# OPT-NOT: call i64 @rb_id2sym
# OPT: call i64 @rb_thread_local_aref
# OPT-NOT: call i64 @rb_id2sym
# OPT{LITERAL}: }

sig {params(thread: Thread, key: T.untyped).returns(T.untyped)}
def thread_aref(thread, key)
  thread[key]
end

# INITIAL-LABEL: define internal i64 @"func_Object#11thread_aref
# INITIAL: call i64 @sorbet_Thread_square_br
# INITIAL{LITERAL}: }

sig {params(thread: Thread, val: T.untyped).returns(T.untyped)}
def thread_aset_constant(thread, val)
  thread[:my_key] = val
end

# INITIAL-LABEL: define internal i64 @"func_Object#20thread_aset_constant
# INITIAL: call i64 @sorbet_Thread_square_br_eq_symarg
# INITIAL{LITERAL}: }

# OPT-LABEL: define internal i64 @"func_Object#20thread_aset_constant
# OPT-NOT: call i64 @rb_id2sym
# OPT: call i64 @sorbet_Thread_square_br_eq_symarg
# OPT-NOT: call i64 @rb_id2sym
# OPT{LITERAL}: }

sig {params(thread: Thread, key: T.untyped, val: T.untyped).returns(T.untyped)}
def thread_aset(thread, key, val)
  thread[key] = val
end

# INITIAL-LABEL: define internal i64 @"func_Object#11thread_aset
# INITIAL: call i64 @sorbet_Thread_square_br_eq
# INITIAL{LITERAL}: }

current = thread_current
p(current.class)

p(thread_aset_constant(current, 'my value'))
p(thread_aset(current, 'another key', 646))

p(thread_aref_constant(current))
p(thread_aref(current, 'another key'))
