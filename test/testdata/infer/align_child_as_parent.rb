# typed: strict
module Mod
  A = type_member
   sig(
      blk: T.proc(a: A).returns(A),
  )
  .returns(NilClass)
  def foo(&blk)
  end
end

class Baz
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
