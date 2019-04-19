class T::Private::Methods::SigBuilder
  Sorbet.sig {params(params: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def type_parameters(*params); end

  Sorbet.sig {returns(T::Private::Methods::SigBuilder)}
  def generated; end

  Sorbet.sig {returns(T::Private::Methods::SigBuilder)}
  def abstract; end

  Sorbet.sig {returns(T::Private::Methods::SigBuilder)}
  def implementation; end

  Sorbet.sig {params(allow_incompatible: T::Boolean)}.returns(T::Private::Methods::SigBuilder)}
  def override(allow_incompatible: false); end

  Sorbet.sig {returns(T::Private::Methods::SigBuilder)}
  def overridable; end

  Sorbet.sig {params(claz: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def bind(claz); end

  Sorbet.sig {params(params: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def params(params); end

  Sorbet.sig {params(type: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def returns(type); end

  Sorbet.sig {returns(T::Private::Methods::SigBuilder)}
  def void; end

  Sorbet.sig {params(notify: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def soft(notify:); end

  Sorbet.sig {params(arg: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def checked(arg); end
end
