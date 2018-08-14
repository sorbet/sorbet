# typed: true
class Float < Numeric
  DIG = T.let(T.unsafe(nil), Integer)
  EPSILON = T.let(T.unsafe(nil), Float)
  INFINITY = T.let(T.unsafe(nil), Float)
  MANT_DIG = T.let(T.unsafe(nil), Integer)
  MAX = T.let(T.unsafe(nil), Float)
  MAX_10_EXP = T.let(T.unsafe(nil), Integer)
  MAX_EXP = T.let(T.unsafe(nil), Integer)
  MIN = T.let(T.unsafe(nil), Float)
  MIN_10_EXP = T.let(T.unsafe(nil), Integer)
  MIN_EXP = T.let(T.unsafe(nil), Integer)
  NAN = T.let(T.unsafe(nil), Float)
  RADIX = T.let(T.unsafe(nil), Integer)
  ROUNDS = T.let(T.unsafe(nil), Integer)

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
  .returns(BigDecimal)
  def %(arg0); end

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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def *(arg0); end

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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def **(arg0); end

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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  Sorbet.sig.returns(Float)
  def +@(); end

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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  Sorbet.sig.returns(Float)
  def -@(); end

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
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

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

  Sorbet.sig.returns(Float)
  def abs(); end

  Sorbet.sig.returns(Float)
  def abs2(); end

  Sorbet.sig.returns(T.any(Integer, Float))
  def angle(); end

  Sorbet.sig.returns(T.any(Integer, Float))
  def arg(); end

  Sorbet.sig.returns(Integer)
  def ceil(); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([Float, Float])
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns([Float, Float])
  def coerce(arg0); end

  Sorbet.sig.returns(Float)
  def conj(); end

  Sorbet.sig.returns(Float)
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
  def eql?(arg0); end

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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def fdiv(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def finite?(); end

  Sorbet.sig.returns(Integer)
  def floor(); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def imag(); end

  Sorbet.sig.returns(Integer)
  def imaginary(); end

  Sorbet.sig.returns(Object)
  def infinite?(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(Float)
  def magnitude(); end

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
  .returns(BigDecimal)
  def modulo(arg0); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  Sorbet.sig.returns(Float)
  def next_float(); end

  Sorbet.sig.returns(Integer)
  def numerator(); end

  Sorbet.sig.returns(T.any(Integer, Float))
  def phase(); end

  Sorbet.sig.returns(Float)
  def prev_float(); end

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

  Sorbet.sig.returns(Float)
  def real(); end

  Sorbet.sig.returns(TrueClass)
  def real?(); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float))
  def round(arg0=_); end

  Sorbet.sig.returns(Complex)
  def to_c(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig.returns(Integer)
  def to_i(); end

  Sorbet.sig.returns(Integer)
  def to_int(); end

  Sorbet.sig.returns(Rational)
  def to_r(); end

  Sorbet.sig.returns(String)
  def to_s(); end

  Sorbet.sig.returns(Integer)
  def truncate(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
