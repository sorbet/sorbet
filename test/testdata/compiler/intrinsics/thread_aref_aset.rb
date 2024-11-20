# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {returns(Thread)}
def thread_current
  Thread.current
end



sig {returns(Thread)}
def thread_main
  Thread.main
end



sig {params(thread: Thread).returns(T.untyped)}
def thread_aref_constant(thread)
  thread[:my_key]
end



sig {params(thread: Thread, key: T.untyped).returns(T.untyped)}
def thread_aref(thread, key)
  thread[key]
end


sig {params(thread: Thread, val: T.untyped).returns(T.untyped)}
def thread_aset_constant(thread, val)
  thread[:my_key] = val
end



sig {params(thread: Thread, key: T.untyped, val: T.untyped).returns(T.untyped)}
def thread_aset(thread, key, val)
  thread[key] = val
end


current = thread_current
p(current.class)

p(thread_aset_constant(current, 'my value'))
p(thread_aset(current, 'another key', 646))

p(thread_aref_constant(current))
p(thread_aref(current, 'another key'))
