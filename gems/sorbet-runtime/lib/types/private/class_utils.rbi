# typed: true


module T::Private::ClassUtils
  sig {params(mod: Module, name: Symbol).returns(Symbol)}
  def self.visibility_method_name(mod, name); end

  sig do
    params(
      mod: Module,
      name: Symbol,
      visibility: Symbol,
      method: T.nilable(T.any(Proc, Method, UnboundMethod)),
      ruby2_keywords: T.any(Symbol, T::Boolean),
      block: T.nilable(Proc)
    )
      .returns(NilClass)
  end
  def self.def_with_visibility(mod, name, visibility, method=nil, ruby2_keywords: :detect, &block)
  end

  sig do
    params(
      original_method: UnboundMethod,
      mod: Module,
      name: Symbol,
      ruby2_keywords: T.any(Symbol, T::Boolean),
      blk: T.untyped
    )
      .returns(NilClass)
  end
  def self.replace_method(original_method, mod, name, ruby2_keywords: :detect, &blk); end
end
