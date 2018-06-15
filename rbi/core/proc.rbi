# typed: true
class Proc < Object
  sig.returns(Integer)
  def arity(); end

  sig.returns(Binding)
  def binding(); end

  sig(
      arg0: BasicObject,
  )
  .returns(T.untyped)
  def call(*arg0); end

  sig(
      arity: Integer,
  )
  .returns(Proc)
  def curry(arity=_); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def lambda(); end

  sig.returns(T::Array[[Symbol, Symbol]])
  def parameters(); end

  sig.returns([String, Integer])
  def source_location(); end

  sig.returns(T.self_type)
  def to_proc(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(String)
  def inspect(); end
end
