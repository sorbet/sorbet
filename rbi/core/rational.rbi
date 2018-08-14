# typed: true
class Rational < Numeric
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  Sorbet.sig.returns(Rational)
  def +@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  Sorbet.sig.returns(Rational)
  def -@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  Sorbet.sig.returns(Rational)
  def abs(); end

  Sorbet.sig.returns(Rational)
  def abs2(); end

  Sorbet.sig.returns(Numeric)
  def angle(); end

  Sorbet.sig.returns(Numeric)
  def arg(); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def ceil(arg0=_); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns([Rational, Rational])
  Sorbet.sig(
      arg0: Float,
  )
  .returns([Float, Float])
  Sorbet.sig(
      arg0: Rational,
  )
  .returns([Rational, Rational])
  Sorbet.sig(
      arg0: Complex,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  Sorbet.sig.returns(Rational)
  def conj(); end

  Sorbet.sig.returns(Rational)
  def conjugate(); end

  Sorbet.sig.returns(Integer)
  def denominator(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  Sorbet.sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Float)
  def fdiv(arg0); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def floor(arg0=_); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def imag(); end

  Sorbet.sig.returns(Integer)
  def imaginary(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  Sorbet.sig.returns(Integer)
  def numerator(); end

  Sorbet.sig.returns(Numeric)
  def phase(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
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
  def rationalize(arg0=_); end

  Sorbet.sig.returns(Rational)
  def real(); end

  Sorbet.sig.returns(TrueClass)
  def real?(); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def round(arg0=_); end

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

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  def truncate(arg0=_); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
