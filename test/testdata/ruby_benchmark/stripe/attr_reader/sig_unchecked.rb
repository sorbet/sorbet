# frozen_string_literal: true
# typed: true
# compiled: true

class AttrReaderSigUnchecked
  extend T::Sig
  sig {params(foo: Integer).void}
  def initialize(foo)
    @foo = foo
  end

  sig {returns(Integer).checked(:never)}
  attr_reader :foo
end

x = AttrReaderSigUnchecked.new(1248)

i = 0
while i < 10_000_000

  x.foo

  i += 1
end

puts i
puts x.foo
