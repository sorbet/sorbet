# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Array[T.untyped])}
def keys(x)
  x.keys
end


sig {params(x: T::Hash[T.untyped, T.untyped]).returns(T::Array[T.untyped])}
def values(x)
  x.values
end


p keys({a: 1, b: 2})
p values({a: 1, b: 2})

class HashSubclass < Hash
  def keys
    ["something else"]
  end

  def values
    ["a complete fiction"]
  end
end

p keys(HashSubclass.new)
p values(HashSubclass.new)
