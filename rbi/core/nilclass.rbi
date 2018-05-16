# typed: true
class NilClass < Object
  sig(
      obj: BasicObject,
  )
  .returns(FalseClass)
  def &(obj); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ^(obj); end

  sig.returns(Rational)
  def rationalize(); end

  sig.returns([])
  def to_a(); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(T.untyped)
  def to_h(); end

  sig.returns(Rational)
  def to_r(); end

  sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def |(obj); end
end
