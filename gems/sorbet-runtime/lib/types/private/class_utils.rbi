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
      block: T.nilable(Proc)
    )
      .returns(NilClass)
  end
  def self.def_with_visibility(mod, name, visibility, method=nil, &block)
  end

  sig do
    params(
      original_method: UnboundMethod,
      mod: Module,
      name: Symbol,
      blk: T.untyped
    )
      .returns(NilClass)
  end
  def self.replace_method(original_method, mod, name, &blk); end
end
