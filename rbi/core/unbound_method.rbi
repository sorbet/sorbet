# typed: strict
class UnboundMethod
  sig {returns(Integer)}
  def arity; end

  sig {params(obj: BasicObject).returns(Method)}
  def bind(obj); end

  sig {returns(Symbol)}
  def name; end

  sig {returns(Module)}
  def owner; end

  sig {returns(T::Array[[Symbol, Symbol]])}
  def parameters; end

  sig {returns(T.nilable([String, Integer]))}
  def source_location; end

  sig {returns(T.nilable(UnboundMethod))}
  def super_method; end
end
