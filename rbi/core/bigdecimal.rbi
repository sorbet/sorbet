# typed: core
class BigDecimal < Numeric
  BASE = T.let(T.unsafe(nil), Integer)
  EXCEPTION_ALL = T.let(T.unsafe(nil), Integer)
  EXCEPTION_INFINITY = T.let(T.unsafe(nil), Integer)
  EXCEPTION_OVERFLOW = T.let(T.unsafe(nil), Integer)
  EXCEPTION_UNDERFLOW = T.let(T.unsafe(nil), Integer)
  EXCEPTION_ZERODIVIDE = T.let(T.unsafe(nil), Integer)
  INFINITY = T.let(T.unsafe(nil), BigDecimal)
  NAN = T.let(T.unsafe(nil), BigDecimal)
  ROUND_CEILING = T.let(T.unsafe(nil), Integer)
  ROUND_DOWN = T.let(T.unsafe(nil), Integer)
  ROUND_FLOOR = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_DOWN = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_EVEN = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_UP = T.let(T.unsafe(nil), Integer)
  ROUND_MODE = T.let(T.unsafe(nil), Integer)
  ROUND_UP = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_FINITE = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_ZERO = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_FINITE = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_ZERO = T.let(T.unsafe(nil), Integer)

  sig do
    params(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
    )
    .void
  end
  def initialize(initial, digits=0); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(BigDecimal)
  end
  def %(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
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
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def **(arg0); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
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

  sig {returns(BigDecimal)}
  def +@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
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

  sig {returns(BigDecimal)}
  def -@(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
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

  sig {returns(String)}
  def _dump(); end

  sig {returns(BigDecimal)}
  def abs(); end

  sig {returns(BigDecimal)}
  def abs2(); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def add(arg0, arg1); end

  sig {returns(Numeric)}
  def angle(); end

  sig {returns(Numeric)}
  def arg(); end

  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(BigDecimal)
  end
  def ceil(digits=0); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([BigDecimal, BigDecimal])
  end
  def coerce(arg0); end

  sig {returns(BigDecimal)}
  def conj(); end

  sig {returns(BigDecimal)}
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

  sig {returns(Integer)}
  def exponent(); end

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

  sig {returns(BigDecimal)}
  def fix(); end

  sig {returns(Integer)}
  sig do
    params(
      digits: Integer
    )
    .returns(BigDecimal)
  end
  def floor(digits=0); end

  sig {returns(BigDecimal)}
  def frac(); end

  sig {returns(Integer)}
  def hash(); end

  sig {returns(Integer)}
  def imag(); end

  sig {returns(Integer)}
  def imaginary(); end

  sig {returns(T.nilable(Integer))}
  def infinite?(); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(BigDecimal)}
  def magnitude(); end

  sig do
    params(
        arg0: Numeric,
    )
    .returns(BigDecimal)
  end
  def modulo(arg0); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def mult(arg0, arg1); end

  sig {returns(T::Boolean)}
  def nan?(); end

  sig {returns(Object)}
  def nonzero?(); end

  sig {returns(Integer)}
  def numerator(); end

  sig {returns(Numeric)}
  def phase(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  def power(arg0); end

  sig {returns([Integer, Integer])}
  def precs(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
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

  sig {returns(BigDecimal)}
  def real(); end

  sig {returns(TrueClass)}
  def real?(); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns(BigDecimal)
  end
  def remainder(arg0); end

  sig {returns(Integer)}
  sig do
    params(
        n: Integer,
        mode: T.any(Integer, Symbol),
    )
    .returns(BigDecimal)
  end
  def round(n=0, mode=T.unsafe(nil)); end

  sig {returns(Integer)}
  def sign(); end

  sig {returns([Integer, String, Integer, Integer])}
  def split(); end

  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  def sqrt(arg0); end

  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
        arg1: Integer,
    )
    .returns(BigDecimal)
  end
  def sub(arg0, arg1); end

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
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
  end
  def truncate(arg0=T.unsafe(nil)); end

  sig {returns(T::Boolean)}
  def zero?(); end
end
