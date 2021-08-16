# typed: true

class A < T::Struct
  const :foo, T::Array[{foo: Integer}]
end

A.new(foo: [])
