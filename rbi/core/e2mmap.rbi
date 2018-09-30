# typed: strict

module Exception2MessageMapper
  sig {params(err: Exception, rest: T.untyped).void}
  def Raise(err, *rest); end
  alias_method(:Fail, :Raise)
  alias_method(:fail, :Raise)

  sig {params(c: Class, m: String).void}
  def def_e2message(c, m); end

  sig {params(c: Symbol, m: String, s: Class).void}
  def def_exception(c, m, s = StandardError); end
end
