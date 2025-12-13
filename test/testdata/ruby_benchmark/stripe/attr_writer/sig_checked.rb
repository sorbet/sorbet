# typed: true
# frozen_string_literal: true
# compiled: true

class AttrWriterSigChecked
  extend T::Sig

  sig {params(foo: Integer).void}
  def initialize(foo)
    @foo = foo
  end

  def foo
    @foo
  end

  sig {params(foo: Integer).void}
  attr_writer :foo
end

x = AttrWriterSigChecked.new(97)

i = 0
while i < 10_000_000
  x.foo = i
  i += 1
end

puts i
puts x.foo
