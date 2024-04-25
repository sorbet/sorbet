# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped).returns(T.untyped)}
def one_arg(hash, key)
  hash.delete(key)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7one_arg"
# INITIAL: call i64 @sorbet_rb_hash_delete_m
# INITIAL{LITERAL}: }

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, blk: T.untyped).returns(T.untyped)}
def block_arg(hash, key, &blk)
  hash.delete(key) do |x|
    yield
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#9block_arg"
# INITIAL-NOT: call i64 @sorbet_rb_hash_delete_m
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak(i64 (i64)* @forward_sorbet_rb_hash_delete_m_withBlock
# INITIAL{LITERAL}: }

h = {key: 5, otherkey: 99}
p one_arg(h, :key)
p h

h = {key: 6, otherkey: 100}
x = block_arg(h, :key) do |x|
  p "don't call"
end
p x
p h

h = {key: 7}
y = block_arg(h, :otherkey) do |x|
  p "called block"
end
p y
p h
