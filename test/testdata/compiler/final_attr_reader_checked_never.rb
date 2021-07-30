# frozen_string_literal: true
# typed: true
# compiled: true

# The compiler has special handling for both final methods (direct calls) and
# `.checked(:never)` attr_reader's.
#
# One of these has to win out over the other:
# - Either we have to skip direct final calls (because there is only an IVAR
#   method entry, which doesn't have a C func to call),
# - Or we have to skip registering the method as an IVAR method (so that the C
#   function can be generated, and have direct final calls).
#
# It is harder to do the first, because "can be defined as an attr_reader
# method" is only present on the MethodDef / keep_def call, not on the symbol.
# Skipping direct final calls would require having the isAttrReader flag on the
# symbol, which we hesitate to do.

class FinalAttrReaderSigUnchecked
  extend T::Sig
  sig {params(foo: Integer).void}
  def initialize(foo)
    @foo = foo
  end

  sig(:final) {returns(Integer).checked(:never)}
  attr_reader :foo
end

x = FinalAttrReaderSigUnchecked.new(247)
puts x.foo
