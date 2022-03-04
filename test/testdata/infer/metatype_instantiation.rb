# typed: true

a = T::Array[{foo: Integer}].new
a << {foo: 1}

b = T::Array[T::Array[{foo: Integer}]].new
b << a

T::Array[{foo: Integer.new}].new # error: Unexpected bare `Integer` value found in type position
T::Array[{foo: 3}].new # error: Unexpected bare `Integer(3)` value found in type position

# Make sure we don't mutate the types when unwrapping
t = [Integer]
T::Array[t].new
T.assert_type!(t, [T.class_of(Integer)])
T::Hash[t, t].new
T.assert_type!(t, [T.class_of(Integer)])
