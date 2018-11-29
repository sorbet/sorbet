# typed: true
class Rational < Numeric
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
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
    .returns(Rational)
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
    .returns(Rational)
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
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Numeric)
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
    .returns(Rational)
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
    .returns(Rational)
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

  sig {returns(Rational)}
  def +@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
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

  sig {returns(Rational)}
  def -@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
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

  sig {returns(Rational)}
  def abs(); end

  sig {returns(Rational)}
  def abs2(); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  sig {returns(Integer)}
  sig do
    params(
        digits: Integer,
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Float,
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end

  sig {returns(Rational)}
  def conj(); end

  sig {returns(Rational)}
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
    .returns(Float)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Float)
  end
  def fdiv(arg0); end

  sig {returns(Integer)}
  sig do
    params(
        digits: Integer,
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  sig {returns(String)}
  def inspect(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(Numeric)}
  def phase(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
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
    .returns(Rational)
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

  sig {returns(Rational)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Numeric)
  end
  def round(arg0=T.unsafe(nil)); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(Integer)}
  def to_i(); end

  sig {returns(Rational)}
  def to_r(); end

  sig {returns(String)}
  def to_s(); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
  end
  def truncate(arg0=T.unsafe(nil)); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def zero?(); end
end
