# typed: true
module Math
  E = T.let(T.unsafe(nil), Float)
  PI = T.let(T.unsafe(nil), Float)

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.acos(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.acosh(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.asin(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.asinh(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atan(x); end

  Sorbet.sig(
      y: T.any(Integer, Float, Rational, BigDecimal),
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atan2(y, x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.atanh(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cbrt(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cos(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.cosh(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.erf(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.erfc(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.exp(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def self.frexp(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.gamma(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
      y: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.hypot(x, y); end

  Sorbet.sig(
      fraction: T.any(Integer, Float, Rational, BigDecimal),
      exponent: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.ldexp(fraction, exponent); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(T.any(Integer, Float))
  def self.lgamma(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
      base: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log(x, base=T.unsafe(nil)); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log10(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.log2(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sin(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sinh(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.sqrt(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.tan(x); end

  Sorbet.sig(
      x: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(Float)
  def self.tanh(x); end
end
