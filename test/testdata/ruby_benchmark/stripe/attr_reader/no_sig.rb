# frozen_string_literal: true
# typed: true
# compiled: true

class AttrReaderNoSig
  extend T::Sig
  sig {params(foo: Integer).void}
  def initialize(foo)
    @foo = foo
  end

  # no sig
  attr_reader :foo
end

x = AttrReaderNoSig.new(1248)

i = 0
while i < 10_000_000

  x.foo

  i += 1
end

puts i
puts x.foo
