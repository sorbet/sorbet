# @typed

class Main
  sig(
    msg: T.nilable(String),
    opts: T::Hash[Symbol, T.untyped],
  )
  .returns(NilClass)
  def check(msg=nil, opts={}); end

  def with_opts(t)
    check(
      storytime: 4
    )
  end
end
