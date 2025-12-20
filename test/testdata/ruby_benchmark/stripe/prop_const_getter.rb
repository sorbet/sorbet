# frozen_string_literal: true
# typed: true
# compiled: true

class MyStruct < T::Struct
  const :foo, Integer
end

my_struct = MyStruct.new(foo: 430)

i = 0
while i < 10_000_000

  my_struct.foo

  i += 1
end

puts i
puts my_struct.foo
