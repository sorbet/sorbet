# typed: strict

module Exception2MessageMapper
  Sorbet.sig(err: Exception, rest: T.untyped).void
  def Raise(err, *rest); end
  alias_method(:Fail, :Raise)
  alias_method(:fail, :Raise)

  Sorbet.sig(c: Class, m: String).void
  def def_e2message(c, m); end

  Sorbet.sig(c: Symbol, m: String, s: Class).void
  def def_exception(c, m, s = StandardError); end
end
