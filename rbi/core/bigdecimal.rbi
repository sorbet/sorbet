# typed: true
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

  Sorbet.sig(
    initial: T.any(Integer, Float, Rational, BigDecimal, String),
    digits: Integer,
  )
  .void
  def initialize(initial, digits=0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(BigDecimal)
  def %(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
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
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def **(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  Sorbet.sig.returns(BigDecimal)
  def +@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  Sorbet.sig.returns(BigDecimal)
  def -@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
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

  Sorbet.sig.returns(String)
  def _dump(); end

  Sorbet.sig.returns(BigDecimal)
  def abs(); end

  Sorbet.sig.returns(BigDecimal)
  def abs2(); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def add(arg0, arg1); end

  Sorbet.sig.returns(Numeric)
  def angle(); end

  Sorbet.sig.returns(Numeric)
  def arg(); end

  Sorbet.sig.returns(Integer)
  def ceil(); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns([BigDecimal, BigDecimal])
  def coerce(arg0); end

  Sorbet.sig.returns(BigDecimal)
  def conj(); end

  Sorbet.sig.returns(BigDecimal)
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

  Sorbet.sig.returns(Integer)
  def exponent(); end

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

  Sorbet.sig.returns(BigDecimal)
  def fix(); end

  Sorbet.sig.returns(Integer)
  def floor(); end

  Sorbet.sig.returns(BigDecimal)
  def frac(); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def imag(); end

  Sorbet.sig.returns(Integer)
  def imaginary(); end

  Sorbet.sig.returns(T.nilable(Integer))
  def infinite?(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(BigDecimal)
  def magnitude(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def mult(arg0, arg1); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def nan?(); end

  Sorbet.sig.returns(Object)
  def nonzero?(); end

  Sorbet.sig.returns(Integer)
  def numerator(); end

  Sorbet.sig.returns(Numeric)
  def phase(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def power(arg0); end

  Sorbet.sig.returns([Integer, Integer])
  def precs(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def quo(arg0); end

  Sorbet.sig.returns(BigDecimal)
  def real(); end

  Sorbet.sig.returns(TrueClass)
  def real?(); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
  )
  .returns(BigDecimal)
  def remainder(arg0); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  def round(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def sign(); end

  Sorbet.sig.returns([Integer, String, Integer, Integer])
  def split(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(BigDecimal)
  def sqrt(arg0); end

  Sorbet.sig(
      arg0: T.any(Integer, Float, Rational, BigDecimal),
      arg1: Integer,
  )
  .returns(BigDecimal)
  def sub(arg0, arg1); end

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
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  def truncate(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
