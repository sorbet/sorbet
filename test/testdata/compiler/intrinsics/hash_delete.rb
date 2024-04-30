# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped).returns(T.untyped)}
def one_arg(hash, key)
  hash.delete(key)
end


sig {params(hash: T::Hash[T.untyped, T.untyped], key: T.untyped, blk: T.untyped).returns(T.untyped)}
def block_arg(hash, key, &blk)
  hash.delete(key) do |x|
    yield
  end
end


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
