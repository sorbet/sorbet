# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

extend T::Sig

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped).returns(T.untyped)}
def one_arg(hash, key)
  hash.fetch(key)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7one_arg"
# INITIAL: call i64 @sorbet_rb_hash_fetch_m
# INITIAL{LITERAL}: }

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, default: T.untyped).returns(T.untyped)}
def two_arg(hash, key, default)
  hash.fetch(key, default)
end

# INITIAL-LABEL: define internal i64 @"func_Object#7two_arg"
# INITIAL: call i64 @sorbet_rb_hash_fetch_m
# INITIAL{LITERAL}: }

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, blk: T.untyped).returns(T.untyped)}
def block_arg(hash, key, &blk)
  hash.fetch(key) do |x|
    yield
  end
end

# INITIAL-LABEL: define internal i64 @"func_Object#9block_arg"
# INITIAL: call i64 @sorbet_callIntrinsicInlineBlock_noBreak({{.*sorbet_rb_hash_fetch_m_withBlock}}
# INITIAL-NOT: call i64 @sorbet_rb_hash_fetch_m
# INITIAL{LITERAL}: }

p one_arg({key: 5}, :key)
p two_arg({key: 7}, :key, :default)
p two_arg({key: 8}, :otherkey, :default)
x = block_arg({key: 6}, :key) do |x|
  p "don't call"
end
p x
y = block_arg({key: 6}, :otherkey) do |x|
  p "called block"
end
p y
  
