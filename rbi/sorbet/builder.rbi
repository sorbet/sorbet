class T::Private::Methods::Builder
  Sorbet.sig {params(params: T.untyped).returns(T::Private::Methods::Builder)}
  def type_parameters(*params); end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def generated; end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def abstract; end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def implementation; end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def override; end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def overridable; end

  Sorbet.sig {params(claz: T.untyped).returns(T::Private::Methods::Builder)}
  def bind(claz); end

  Sorbet.sig {params(params: T.untyped).returns(T::Private::Methods::Builder)}
  def params(**params); end

  Sorbet.sig {params(type: T.untyped).returns(T::Private::Methods::Builder)}
  def returns(type); end

  Sorbet.sig {returns(T::Private::Methods::Builder)}
  def void; end

  Sorbet.sig {params(params: T.untyped).returns(T::Private::Methods::Builder)}
  def soft(**params); end

  Sorbet.sig {params(arg: T.untyped).returns(T::Private::Methods::Builder)}
  def checked(arg); end
end
