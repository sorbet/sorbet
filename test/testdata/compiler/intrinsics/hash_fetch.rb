# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped).returns(T.untyped)}
def one_arg(hash, key)
  hash.fetch(key)
end


sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, default: T.untyped).returns(T.untyped)}
def two_arg(hash, key, default)
  hash.fetch(key, default)
end


sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, blk: T.untyped).returns(T.untyped)}
def block_arg(hash, key, &blk)
  hash.fetch(key) do |x|
    yield
  end
end


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
  
