# typed: true
module Math
  E = T.let(T.unsafe(nil), Float)
  PI = T.let(T.unsafe(nil), Float)

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.acos(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.acosh(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.asin(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.asinh(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atan(x); end

  sig do
    params(
        y: T.any(Integer, Float, Rational, BigDecimal),
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atan2(y, x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.atanh(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cbrt(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cos(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.cosh(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.erf(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.erfc(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.exp(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def self.frexp(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.gamma(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
        y: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.hypot(x, y); end

  sig do
    params(
        fraction: T.any(Integer, Float, Rational, BigDecimal),
        exponent: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.ldexp(fraction, exponent); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(T.any(Integer, Float))
  end
  def self.lgamma(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
        base: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log(x, base=T.unsafe(nil)); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log10(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.log2(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sin(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sinh(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.sqrt(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.tan(x); end

  sig do
    params(
        x: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(Float)
  end
  def self.tanh(x); end
end
