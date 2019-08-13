# typed: strict
# frozen_string_literal: true

# Ruby model extensions.
#
# `sorbet-static` relies on some definitions that exist at compile-time in the RBIs.
# Some of these definitions will never exist in real Ruby code but `sorbet-runtime` will need them to work.
# We define them here.

module Enumerable
  extend T::Sig
  extend T::Generic
  Elem = type_member(:out)

  # Sorbet define `Enumerable` as an interface where `each` should be implemented
  # by the clients with `implementation`.
  # We define it here so `sorbet-runtime` knows what's `each` expected signature.

  sig do
    abstract.
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def each(&blk); end
end
