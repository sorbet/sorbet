# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

# [] is not the only method that we need to be careful about, but it's a
# reasonable starting place.

class Arraylike < Array
  def [](key)
    "overridden"
  end
end

sig {params(array: T::Array[T.untyped]).returns(T.untyped)}
def ref(array)
  array[0]
end

a = Arraylike["default"]

p ref(a)
