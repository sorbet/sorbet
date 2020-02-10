# typed: strict

require 'sorbet-runtime'

class MyEnumerable
  extend T::Generic
  extend T::Sig
  include Enumerable

  Elem = type_member

  sig do
    override.
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def each(&blk)
  end
end

module MyCoVariant
  extend T::Generic

  Elem = type_member(:out)
end

module MyContraVariant
  extend T::Generic

  Elem = type_member(:in)
end
