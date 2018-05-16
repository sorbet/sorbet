# typed: true
class Integer < Numeric
  sig(
      arg0: Integer,
  )
  .returns(Integer)
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
  .returns(Integer)
  def &(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
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
  .returns(Integer)
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

  sig.returns(Integer)
  def +@(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
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

  sig.returns(Integer)
  def -@(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
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
  .returns(Integer)
  def <<(arg0); end

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
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ===(arg0); end

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

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def >>(arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  sig(
      arg0: Rational,
  )
  .returns(Integer)
  sig(
      arg0: Float,
  )
  .returns(Integer)
  sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def [](arg0); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def ^(arg0); end

  sig.returns(Integer)
  def abs(); end

  sig.returns(Integer)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def bit_length(); end

  sig.returns(Integer)
  def ceil(); end

  sig(
      arg0: Encoding,
  )
  .returns(String)
  def chr(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def coerce(arg0); end

  sig.returns(Integer)
  def conj(); end

  sig.returns(Integer)
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
      limit: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig(
      limit: Integer,
  )
  .returns(Enumerator[Integer])
  def downto(limit, &blk); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Object,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(arg0); end

  sig.returns(T.any(TrueClass, FalseClass))
  def even?(); end

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
  .returns(BigDecimal)
  sig(
      arg0: Complex,
  )
  .returns(Complex)
  def fdiv(arg0); end

  sig.returns(Integer)
  def floor(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def gcd(arg0); end

  sig(
      arg0: Integer,
  )
  .returns([Integer, Integer])
  def gcdlcm(arg0); end

  sig.returns(Integer)
  def hash(); end

  sig.returns(Integer)
  def imag(); end

  sig.returns(Integer)
  def imaginary(); end

  sig.returns(String)
  def inspect(); end

  sig.returns(TrueClass)
  def integer?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lcm(arg0); end

  sig.returns(Integer)
  def magnitude(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
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
  def next(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def odd?(); end

  sig.returns(Integer)
  def ord(); end

  sig.returns(Numeric)
  def phase(); end

  sig.returns(Integer)
  def pred(); end

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

  sig.returns(Integer)
  def real(); end

  sig.returns(TrueClass)
  def real?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
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
  def remainder(arg0); end

  sig.returns(Integer)
  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0=_); end

  sig.returns(Integer)
  def size(); end

  sig.returns(Integer)
  def succ(); end

  sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig.returns(Enumerator[Integer])
  def times(&blk); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Float)
  def to_f(); end

  sig.returns(Integer)
  def to_i(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Rational)
  def to_r(); end

  sig.returns(String)
  def to_s(); end

  sig.returns(Integer)
  def truncate(); end

  sig(
      arg0: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  sig(
      arg0: Integer,
  )
  .returns(Enumerator[Integer])
  def upto(arg0, &blk); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  sig(
      arg0: Integer,
  )
  .returns(Integer)
  def |(arg0); end

  sig.returns(Integer)
  def ~(); end
end
