# @typed
module Mod
  A = T.type
   sig(
      blk: T::Proc[[A], returns: A],
  )
  .returns(NilClass)
  def foo(&blk)
  end
end

class Baz
  include Mod
  B = T.type
  C = T.type
  A = T.type

  def bla()
    foo do |s| # this triggers alignBaseTypeArgs where we have to adapt parent to child. Child has more tparams than parent
      s
    end
  end
end
