# typed: true
# frozen_string_literal: true
# compiled: true

class AttrWriterNoSig
  extend T::Sig

  sig {params(foo: Integer).void}
  def initialize(foo)
    @foo = foo
  end

  def foo
    @foo
  end

  # no sig
  attr_writer :foo
end

x = AttrWriterNoSig.new(97)

i = 0
while i < 10_000_000
  x.foo = i
  i += 1
end

puts i
puts x.foo
