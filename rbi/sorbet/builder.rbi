# typed: __STDLIB_INTERNAL
class T::Private::Methods::DeclBuilder
  sig {params(params: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def type_parameters(*params); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def abstract; end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def final; end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def implementation; end

  sig {params(allow_incompatible: T::Boolean).returns(T::Private::Methods::DeclBuilder)}
  def override(allow_incompatible: false); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def overridable; end

  sig {params(claz: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def bind(claz); end

  sig {params(unused_positional_params: T.anything, params: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def params(*unused_positional_params, **params); end

  sig {params(type: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def returns(type); end

  sig {returns(T::Private::Methods::DeclBuilder)}
  def void; end

  sig {params(args: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def on_failure(*args); end

  sig {params(arg: T.anything).returns(T::Private::Methods::DeclBuilder)}
  def checked(arg); end
end
