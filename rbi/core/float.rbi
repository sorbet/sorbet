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

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def %(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def *(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def **(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end

  sig {returns(Float)}
  def +@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end

  sig {returns(Float)}
  def -@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def /(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def <(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def <=(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  def <=>(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ===(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def >(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def >=(arg0); end

  sig {returns(Float)}
  def abs(); end

  sig {returns(Float)}
  def abs2(); end

  sig {returns(T.any(Integer, Float))}
  def angle(); end

  sig {returns(T.any(Integer, Float))}
  def arg(); end

  sig {returns(Integer)}
  def ceil(); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Float, Float])
  end
  def coerce(arg0); end

  sig {returns(Float)}
  def conj(); end

  sig {returns(Float)}
  def conjugate(); end

  sig {returns(Integer)}
  def denominator(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  def div(arg0); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def divmod(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def equal?(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def fdiv(arg0); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def finite?(); end

  sig {returns(Integer)}
  def floor(); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  sig {returns(Object)}
  def infinite?(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(Float)}
  def magnitude(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def nan?(); end

  sig {returns(Float)}
  def next_float(); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(T.any(Integer, Float))}
  def phase(); end

  sig {returns(Float)}
  def prev_float(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def quo(arg0); end

  sig {returns(Rational)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Rational)
  end
  def rationalize(arg0=T.unsafe(nil)); end

  sig {returns(Float)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float))
  end
  def round(arg0=T.unsafe(nil)); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(Integer)}
  def to_i(); end

  sig {returns(Integer)}
  def to_int(); end

  sig {returns(Rational)}
  def to_r(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(Integer)}
  def truncate(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def zero?(); end
end
