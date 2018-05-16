# typed: true
class Rational < Numeric
  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  sig(
      arg0: Float,
  )
  .returns(Numeric)
  sig(
      arg0: Rational,
  )
  .returns(Numeric)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  sig.returns(Rational)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  sig.returns(Rational)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def /(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Float,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: Rational,
  )
  .returns(T.any(TrueClass, FalseClass))
  sig(
      arg0: BigDecimal,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(Rational)
  def abs(); end

  sig.returns(Rational)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def ceil(arg0=_); end

  sig(
      arg0: Integer,
  )
  .returns([Rational, Rational])
  sig(
      arg0: Float,
  )
  .returns([Float, Float])
  sig(
      arg0: Rational,
  )
  .returns([Rational, Rational])
  sig(
      arg0: Complex,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  sig.returns(Rational)
  def conj(); end

  sig.returns(Rational)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def divmod(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Float)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Float)
  sig(
      arg0: BigDecimal,
  )
  .returns(Float)
  sig(
      arg0: Complex,
  )
  .returns(Float)
  def fdiv(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def floor(arg0=_); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(String)
  def inspect(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig(
      arg0: Integer,
  )
  .returns(Rational)
  sig(
      arg0: Float,
  )
  .returns(Float)
  sig(
      arg0: Rational,
  )
  .returns(Rational)
  sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  sig.returns(Rational)
  sig(
      arg0: Numeric,
  )
  .returns(Rational)
  def rationalize(arg0=_); end

  sig.returns(Rational)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Numeric)
  def round(arg0=_); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Rational)
  def truncate(arg0=_); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
