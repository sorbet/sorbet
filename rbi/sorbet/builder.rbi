# typed: __STDLIB_INTERNAL
class T::Private::Methods::DeclBuilder
  sig {params(params: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def type_parameters(*params); end

  sig do
    params(
      blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
    ).returns(T::Private::Methods::DeclBuilder)
  end
  def abstract(&blk); end

  sig do
    params(
      blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
    ).returns(T::Private::Methods::DeclBuilder)
  end
  def final(&blk); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def implementation; end

  sig do
    params(
      allow_incompatible: T::Boolean,
      blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
    ).returns(T::Private::Methods::DeclBuilder)
  end
  def override(allow_incompatible: false, &blk); end

  sig do
    params(
      blk: T.nilable(T.proc.bind(T::Private::Methods::DeclBuilder).void)
    ).returns(T::Private::Methods::DeclBuilder)
  end
  def overridable(&blk); end

  sig {params(claz: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def bind(claz); end

  sig {params(params: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def params(**params); end

  sig {params(type: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def returns(type); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def void; end

  sig {params(args: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def on_failure(*args); end

  sig {params(arg: T.untyped).returns(T::Private::Methods::DeclBuilder)}
  def checked(arg); end
end
