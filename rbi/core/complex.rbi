# typed: true
class Complex < Numeric
  I = T.let(T.unsafe(nil), Complex)

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  Sorbet.sig.returns(Complex)
  def +@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  Sorbet.sig.returns(Complex)
  def -@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig.returns(Numeric)
  def abs(); end

  Sorbet.sig.returns(Numeric)
  def abs2(); end

  Sorbet.sig.returns(Float)
  def angle(); end

  Sorbet.sig.returns(Float)
  def arg(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns([Complex, Complex])
  def coerce(arg0); end

  Sorbet.sig.returns(Complex)
  def conj(); end

  Sorbet.sig.returns(Complex)
  def conjugate(); end

  Sorbet.sig.returns(Integer)
  def denominator(); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Complex)
  def fdiv(arg0); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def imag(); end

  Sorbet.sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def imaginary(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def magnitude(); end

  Sorbet.sig.returns(Complex)
  def numerator(); end

  Sorbet.sig.returns(Float)
  def phase(); end

  Sorbet.sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def polar(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Complex)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  Sorbet.sig.returns(Rational)
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(Integer, Float, Rational, BigDecimal))
  def real(); end

  Sorbet.sig.returns(FalseClass)
  def real?(); end

  Sorbet.sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def rect(); end

  Sorbet.sig.returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def rectangular(); end

  Sorbet.sig.returns(Complex)
  def to_c(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig.returns(Integer)
  def to_i(); end

  Sorbet.sig.returns(Rational)
  def to_r(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
