# frozen_string_literal: true
# typed: true
# compiled: true

class TestClass < T::Struct
  prop :int, Integer
  prop :str, String
end

i = 0
while i < 10_000_000
  TestClass.new(int: 5, str: "foo")

  i += 1
end

puts i
