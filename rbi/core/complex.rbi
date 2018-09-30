# typed: true
class Complex < Numeric
  I = T.let(T.unsafe(nil), Complex)

  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
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
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
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
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end

  sig {returns(Complex)}
  def +@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end

  sig {returns(Complex)}
  def -@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
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
        arg0: Object,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(arg0); end

  sig {returns(Numeric)}
  def abs(); end

  sig {returns(Numeric)}
  def abs2(); end

  sig {returns(Float)}
  def angle(); end

  sig {returns(Float)}
  def arg(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns([Complex, Complex])
  end
  def coerce(arg0); end

  sig {returns(Complex)}
  def conj(); end

  sig {returns(Complex)}
  def conjugate(); end

  sig {returns(Integer)}
  def denominator(); end

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
        arg0: Numeric,
    )
    .returns(Complex)
  end
  def fdiv(arg0); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def imag(); end

  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def imaginary(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def magnitude(); end

  sig {returns(Complex)}
  def numerator(); end

  sig {returns(Float)}
  def phase(); end

  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def polar(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
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

  sig {returns(T.any(Integer, Float, Rational, BigDecimal))}
  def real(); end

  sig {returns(FalseClass)}
  def real?(); end

  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def rect(); end

  sig {returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])}
  def rectangular(); end

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

  sig {returns(T.any(TrueClass, FalseClass))}
  def zero?(); end
end
