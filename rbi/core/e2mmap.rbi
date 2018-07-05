module Exception2MessageMapper
  sig(err: Exception, rest: T.untyped).void
  def Raise(err, *rest); end
  alias_method(:Fail, :Raise)
  alias_method(:fail, :Raise)

  sig(c: Class, m: String).void
  def def_e2message(c, m); end

  sig(c: Symbol, m: String, s: Class).void
  def def_exception(c, m, s = StandardError); end
end
