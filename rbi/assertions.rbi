# typed: strict
module Kernel
  sig do
    params(
      predicate: BasicObject,
      msg: T.nilable(String),
      opts: T.untyped,
    )
    .returns(NilClass)
  end
  def hard_assert(predicate, msg=nil, **opts); end
end
