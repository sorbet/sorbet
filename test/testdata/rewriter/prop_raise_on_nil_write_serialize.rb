# typed: strict
class Thing < T::Struct
  prop :foo, T.nilable(String), raise_on_nil_write: true
  prop :bar, String
end

obj = Thing.new(bar: 'bar!')

obj.serialize

# this should be acceptable
obj.foo = 'a string'

obj.foo = nil
