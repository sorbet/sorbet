# typed: true
class Integer < Numeric
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
    .returns(Integer)
  end
  def &(arg0); end

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
    .returns(Integer)
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

  sig {returns(Integer)}
  def +@(); end

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

  sig {returns(Integer)}
  def -@(); end

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
    .returns(Integer)
  end
  def <<(arg0); end

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

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def >>(arg0); end

  sig do
    params(
        arg0: Integer,
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
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  def [](arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def ^(arg0); end

  sig {returns(Integer)}
  def abs(); end

  sig {returns(Integer)}
  def abs2(); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  sig {returns(Integer)}
  def bit_length(); end

  sig {returns(Integer)}
  def ceil(); end

  sig do
    params(
        arg0: Encoding,
    )
    .returns(String)
  end
  def chr(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def coerce(arg0); end

  sig {returns(Integer)}
  def conj(); end

  sig {returns(Integer)}
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
        limit: Integer,
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig do
    params(
        limit: Integer,
    )
    .returns(Enumerator[Integer])
  end
  def downto(limit, &blk); end

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

  sig {returns(T.any(TrueClass, FalseClass))}
  def even?(); end

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

  sig {returns(Integer)}
  def floor(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def gcd(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns([Integer, Integer])
  end
  def gcdlcm(arg0); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(TrueClass)}
  def integer?(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def lcm(arg0); end

  sig {returns(Integer)}
  def magnitude(); end

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
  def next(); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def odd?(); end

  sig {returns(Integer)}
  def ord(); end

  sig {returns(Numeric)}
  def phase(); end

  sig {returns(Integer)}
  def pred(); end

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

  sig {returns(Integer)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

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
  def remainder(arg0); end

  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def round(arg0=T.unsafe(nil)); end

  sig {returns(Integer)}
  def size(); end

  sig {returns(Integer)}
  def succ(); end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig {returns(Enumerator[Integer])}
  def times(&blk); end

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

  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Integer])
  end
  def upto(arg0, &blk); end

  sig {returns(T.any(TrueClass, FalseClass))}
  def zero?(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def |(arg0); end

  sig {returns(Integer)}
  def ~(); end
end
