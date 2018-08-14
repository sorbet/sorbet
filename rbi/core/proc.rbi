# typed: true
class Proc < Object
  Sorbet.sig.returns(Integer)
  def arity(); end

  Sorbet.sig.returns(Binding)
  def binding(); end

  Sorbet.sig(
      arg0: BasicObject,
  )
  .returns(T.untyped)
  def call(*arg0); end

  Sorbet.sig(
      arity: Integer,
  )
  .returns(Proc)
  def curry(arity=_); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def lambda(); end

  Sorbet.sig.returns(T::Array[[Symbol, Symbol]])
  def parameters(); end

  Sorbet.sig.returns([String, Integer])
  def source_location(); end

  Sorbet.sig.returns(T.self_type)
  def to_proc(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(String)
  def inspect(); end
end
