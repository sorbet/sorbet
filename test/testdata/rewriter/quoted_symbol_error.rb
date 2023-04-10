# typed: true

class A < T::Struct
  prop :'foo', Integer
  prop :"bar", Integer
  prop :"", Integer
  prop :"#{A.name}", Integer
end

a = A.new(foo: 0, bar: 1) # error: Missing required keyword argument `` for method `A#initialize`
a.foo = '' # error: Assigning a value to `foo` that does not match expected type `Integer`
a.bar = '' # error: Assigning a value to `bar` that does not match expected type `Integer`