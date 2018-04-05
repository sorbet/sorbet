module Kernel
  sig(
    predicate: BasicObject,
    msg: T.nilable(String),
    opts: T.untyped,
  )
  .returns(NilClass)
  def hard_assert(predicate, msg=nil, **opts); end
end
