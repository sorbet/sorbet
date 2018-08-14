# typed: true
class NilClass < Object
  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(FalseClass)
  def &(obj); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ^(obj); end

  Sorbet.sig.returns(Rational)
  def rationalize(); end

  Sorbet.sig.returns([])
  def to_a(); end

  Sorbet.sig.returns(Complex)
  def to_c(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig.returns(T.untyped)
  def to_h(); end

  Sorbet.sig.returns(Rational)
  def to_r(); end

  Sorbet.sig(
      obj: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def |(obj); end
end
