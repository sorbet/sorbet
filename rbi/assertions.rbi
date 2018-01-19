module Kernel
  sig(
    predicate: T.any(TrueClass, FalseClass),
    msg: T.nilable(String),
    opts: T::Hash[Symbol, T.untyped],
  )
  .returns(NilClass)
  def hard_assert(predicate, msg=nil, opts={}); end
end
