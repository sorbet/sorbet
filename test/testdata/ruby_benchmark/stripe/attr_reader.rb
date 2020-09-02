# frozen_string_literal: true
# typed: true
# compiled: true

class MyStruct
  extend T::Sig
  sig {params(foo: Integer).void}
  def initialize(foo:)
    @foo = foo
  end

  attr_reader :foo
end

my_struct = MyStruct.new(foo: 430)

i = 0
while i < 10_000_000

  my_struct.foo

  i += 1
end

puts i
puts my_struct.foo
