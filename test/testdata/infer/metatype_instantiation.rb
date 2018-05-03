# typed: strict

a = T::Array[{foo: Integer}].new
a << {foo: 1}

b = T::Array[T::Array[{foo: Integer}]].new
b << a

T::Array[{foo: Integer.new}].new # error: Unsupported usage of bare type
T::Array[{foo: 3}].new # error: Unsupported usage of literal type

# Make sure we don't mutate the types when unwrapping
t = [Integer]
T::Array[t].new
T.assert_type!(t, [Integer.singleton_class])
T::Hash[t, t].new
T.assert_type!(t, [Integer.singleton_class])
