# typed: true
class NilClass < Object
  sig do
    params(
        obj: BasicObject,
    )
    .returns(FalseClass)
  end
  def &(obj); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ^(obj); end

  sig {returns(Rational)}
  def rationalize(); end

  sig {returns([])}
  def to_a(); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end

  sig {returns(Rational)}
  def to_r(); end

  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def |(obj); end

  sig {returns(TrueClass)}
  def nil?; end
end
