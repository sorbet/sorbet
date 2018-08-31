# typed: true
class Integer < Numeric
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def %(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def &(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
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
  .returns(Numeric)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Numeric)
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
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def +(arg0); end

  Sorbet.sig.returns(Integer)
  def +@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  Sorbet.sig(
      arg0: Complex,
  )
  .returns(Complex)
  def -(arg0); end

  Sorbet.sig.returns(Integer)
  def -@(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
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
  .returns(Integer)
  def <<(arg0); end

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

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def >>(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(Integer)
  def [](arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def ^(arg0); end

  Sorbet.sig.returns(Integer)
  def abs(); end

  Sorbet.sig.returns(Integer)
  def abs2(); end

  Sorbet.sig.returns(Numeric)
  def angle(); end

  Sorbet.sig.returns(Numeric)
  def arg(); end

  Sorbet.sig.returns(Integer)
  def bit_length(); end

  Sorbet.sig.returns(Integer)
  def ceil(); end

  Sorbet.sig(
      arg0: Encoding,
  )
  .returns(String)
  def chr(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  def coerce(arg0); end

  Sorbet.sig.returns(Integer)
  def conj(); end

  Sorbet.sig.returns(Integer)
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
      limit: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  Sorbet.sig(
      limit: Integer,
  )
  .returns(Enumerator[Integer])
  def downto(limit, &blk); end

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

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def even?(); end

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

  Sorbet.sig.returns(Integer)
  def floor(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def gcd(arg0); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns([Integer, Integer])
  def gcdlcm(arg0); end

  Sorbet.sig.returns(Integer)
  def hash(); end

  Sorbet.sig.returns(Integer)
  def imag(); end

  Sorbet.sig.returns(Integer)
  def imaginary(); end

  Sorbet.sig.returns(String)
  def inspect(); end

  Sorbet.sig.returns(TrueClass)
  def integer?(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def lcm(arg0); end

  Sorbet.sig.returns(Integer)
  def magnitude(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def modulo(arg0); end

  Sorbet.sig.returns(Integer)
  def next(); end

  Sorbet.sig.returns(Integer)
  def numerator(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def odd?(); end

  Sorbet.sig.returns(Integer)
  def ord(); end

  Sorbet.sig.returns(Numeric)
  def phase(); end

  Sorbet.sig.returns(Integer)
  def pred(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
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
  def rationalize(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def real(); end

  Sorbet.sig.returns(TrueClass)
  def real?(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Float,
  )
  .returns(Float)
  Sorbet.sig(
      arg0: Rational,
  )
  .returns(Rational)
  Sorbet.sig(
      arg0: BigDecimal,
  )
  .returns(BigDecimal)
  def remainder(arg0); end

  Sorbet.sig.returns(Integer)
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0=T.unsafe(nil)); end

  Sorbet.sig.returns(Integer)
  def size(); end

  Sorbet.sig.returns(Integer)
  def succ(); end

  Sorbet.sig(
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  Sorbet.sig.returns(Enumerator[Integer])
  def times(&blk); end

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

  Sorbet.sig(
      arg0: Integer,
      blk: T.proc(arg0: Integer).returns(BasicObject),
  )
  .returns(Integer)
  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Enumerator[Integer])
  def upto(arg0, &blk); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end

  Sorbet.sig(
      arg0: Integer,
  )
  .returns(Integer)
  def |(arg0); end

  Sorbet.sig.returns(Integer)
  def ~(); end
end
