# typed: true
module Mod
  extend T::Generic
  extend T::Sig

  A = type_member
   sig do
     params(
        blk: T.proc.params(a: A).returns(A),
    )
    .returns(NilClass)
   end
  def foo(&blk)
  end
end

class Baz
  extend T::Generic
  include Mod
  B = type_member
  C = type_member
  A = type_member

  def bla()
    foo do |s| # this triggers alignBaseTypeArgs where we have to adapt parent to child. Child has more tparams than parent
      s
    end
  end
end
