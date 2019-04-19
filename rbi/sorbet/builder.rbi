class Sorbet::Private::Builder
  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def type_parameters(*params); end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def generated; end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def abstract; end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def implementation; end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def override; end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def overridable; end

  Sorbet.sig {params(claz: T.untyped).returns(Sorbet::Private::Builder)}
  def bind(claz); end

  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def params(**params); end

  Sorbet.sig {params(type: T.untyped).returns(Sorbet::Private::Builder)}
  def returns(type); end

  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def void; end

  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def soft(**params); end

  Sorbet.sig {params(arg: T.untyped).returns(Sorbet::Private::Builder)}
  def checked(arg); end
end
