# typed: strict
class Thing
  include T::Props
  prop :foo, T.nilable(String), raise_on_nil_write: true
  prop :bar, T.any(NilClass, String), raise_on_nil_write: true
end

obj = Thing.new
# this should be acceptable, but wouldn't normally be seen
obj.bar = nil

# this should be acceptable
obj.foo = 'a string'

# this should error, since the setter will not allow nil
obj.foo = nil
       #  ^^^ error: Assigning a value to `foo` that does not match expected type `String`
