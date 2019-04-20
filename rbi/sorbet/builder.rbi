class T::Private::Methods::SigBuilder
  sig {params(params: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def type_parameters(*params); end

  sig {returns(T::Private::Methods::SigBuilder)}
  def generated; end

  sig {returns(T::Private::Methods::SigBuilder)}
  def abstract; end

  sig {returns(T::Private::Methods::SigBuilder)}
  def implementation; end

  sig {params(allow_incompatible: T::Boolean).returns(T::Private::Methods::SigBuilder)}
  def override(allow_incompatible: false); end

  sig {returns(T::Private::Methods::SigBuilder)}
  def overridable; end

  sig {params(claz: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def bind(claz); end

  sig {params(params: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def params(params); end

  sig {params(type: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def returns(type); end

  sig {returns(T::Private::Methods::SigBuilder)}
  def void; end

  sig {params(notify: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def soft(notify:); end

  sig {params(arg: T.untyped).returns(T::Private::Methods::SigBuilder)}
  def checked(arg); end
end
