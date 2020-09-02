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

  # This is here to trick the compiler into redefining the method using
  # `attr_reader`, which the VM special cases.
  self.send(:attr_reader, :foo)
end

my_struct = MyStruct.new(foo: 430)

i = 0
while i < 10_000_000

  my_struct.foo

  i += 1
end

puts i
puts my_struct.foo
