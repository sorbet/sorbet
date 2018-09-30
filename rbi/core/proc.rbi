# typed: true
class Proc < Object
  sig {returns(Integer)}
  def arity(); end

  sig {returns(Binding)}
  def binding(); end

  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def call(*arg0); end

  sig do
    params(
        arity: Integer,
    )
    .returns(Proc)
  end
  def curry(arity=T.unsafe(nil)); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def lambda(); end

  sig {returns(T::Array[[Symbol, Symbol]])}
  def parameters(); end

  sig {returns([String, Integer])}
  def source_location(); end

  sig {returns(T.self_type)}
  def to_proc(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(String)}
  def inspect(); end
end
