# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

# [] is not the only method that we need to be careful about, but it's a
# reasonable starting place.

class Hashlike < Hash
  def [](key)
    "overridden"
  end
end

sig {params(hash: T::Hash[T.untyped, T.untyped]).returns(T.untyped)}
def ref(hash)
  hash["key"]
end

h = T.unsafe(Hashlike)["key", "default"]

p ref(h)
