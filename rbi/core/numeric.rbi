# typed: core
class Numeric < Object
  include Comparable

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def %(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end

  sig {returns(Numeric)}
  def +@(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end

  sig do
    params(
      arg0: Numeric,
    )
    .returns(Numeric)
  end
  def *(arg0); end

  sig do
    params(
      arg0: Numeric,
    )
    .returns(Numeric)
  end
  def /(arg0); end

  sig {returns(Numeric)}
  def -@(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def <=(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Integer)
  end
  def <=>(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def >=(arg0); end

  sig {returns(Numeric)}
  def abs(); end

  sig {returns(Numeric)}
  def abs2(); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end

  sig {returns(Numeric)}
  def conj(); end

  sig {returns(Numeric)}
  def conjugate(); end

  sig {returns(Integer)}
  def denominator(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Integer)
  end
  def div(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns([Numeric, Numeric])
  end
  def divmod(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def fdiv(arg0); end

  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

  sig {returns(Complex)}
  def i(); end

  sig {returns(Numeric)}
  def imag(); end

  sig {returns(Numeric)}
  def imaginary(); end

  sig {returns(T::Boolean)}
  def integer?(); end

  sig {returns(Numeric)}
  def magnitude(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float, Rational, BigDecimal))
  end
  def modulo(arg0); end

  sig {returns(T.nilable(T.self_type))}
  def nonzero?(); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(Numeric)}
  def phase(); end

  sig {returns([Numeric, Numeric])}
  def polar(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def quo(arg0); end

  sig {returns(Numeric)}
  def real(); end

  sig {returns(Numeric)}
  def real?(); end

  sig {returns([Numeric, Numeric])}
  def rect(); end

  sig {returns([Numeric, Numeric])}
  def rectangular(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float, Rational, BigDecimal))
  end
  def remainder(arg0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def round(arg0); end

  sig do
    params(
        arg0: Symbol,
    )
    .returns(TypeError)
  end
  def singleton_method_added(arg0); end

  sig do
    params(
        arg0: Numeric,
        blk: T.proc.params(arg0: Numeric).returns(BasicObject),
    )
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Enumerator[Numeric])
  end
  sig do
    params(
        arg0: Numeric,
        arg1: Numeric,
        blk: T.proc.params(arg0: Numeric).returns(BasicObject),
    )
    .returns(Numeric)
  end
  sig do
    params(
        arg0: Numeric,
        arg1: Numeric,
    )
    .returns(Enumerator[Numeric])
  end
  def step(arg0, arg1=T.unsafe(nil), &blk); end

  sig {returns(Complex)}
  def to_c(); end

  sig {returns(Float)}
  def to_f(); end

  sig {returns(Integer)}
  def to_i(); end

  sig {returns(Integer)}
  def to_int(); end

  sig {returns(Integer)}
  def truncate(); end

  sig {returns(T::Boolean)}
  def zero?(); end
end

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
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
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
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
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
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

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
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
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

  sig {returns(T::Boolean)}
  def finite?(); end

  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
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

  sig {returns(T::Boolean)}
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

  sig {params(base: Integer).returns(String)}
  def to_s(base=10); end

  sig {returns(Integer)}
  def truncate(); end

  sig {returns(T::Boolean)}
  def zero?(); end
end

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
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
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
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
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
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
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
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def ceil(digits=0); end

  sig {returns(String)}
  sig do
    params(
        arg0: T.any(Encoding, String),
    )
    .returns(String)
  end
  def chr(arg0=T.unsafe(nil)); end

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
    .returns(T::Boolean)
  end
  def eql?(arg0); end

  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def equal?(arg0); end

  sig {returns(T::Boolean)}
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
  sig do
    params(
      digits: Integer
    )
    .returns(Numeric)
  end
  def floor(digits=0); end

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

  sig {returns(T::Boolean)}
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

  sig {params(base: Integer).returns(String)}
  def to_s(base=10); end

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

  sig {returns(T::Boolean)}
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

class FloatDomainError < RangeError
end

class ZeroDivisionError < StandardError
end

Fixnum = Integer
