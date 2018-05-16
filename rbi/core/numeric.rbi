# typed: true
class Numeric < Object
  include Comparable

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def %(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def +(arg0); end

  sig.returns(Numeric)
  def +@(); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def -(arg0); end

  sig.returns(Numeric)
  def -@(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def <=>(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  sig.returns(Numeric)
  def abs(); end

  sig.returns(Numeric)
  def abs2(); end

  sig.returns(Numeric)
  def angle(); end

  sig.returns(Numeric)
  def arg(); end

  sig.returns(Integer)
  def ceil(); end

  sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  sig.returns(Numeric)
  def conj(); end

  sig.returns(Numeric)
  def conjugate(); end

  sig.returns(Integer)
  def denominator(); end

  sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def div(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def divmod(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def fdiv(arg0); end

  sig.returns(Integer)
  def floor(); end

  sig.returns(Complex)
  def i(); end

  sig.returns(Numeric)
  def imag(); end

  sig.returns(Numeric)
  def imaginary(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def integer?(); end

  sig.returns(Numeric)
  def magnitude(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def modulo(arg0); end

  type_parameters(:Self).sig.returns(T.nilable(T.type_parameter(:Self)))
  def nonzero?(); end

  sig.returns(Integer)
  def numerator(); end

  sig.returns(Numeric)
  def phase(); end

  sig.returns([Numeric, Numeric])
  def polar(); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def quo(arg0); end

  sig.returns(Numeric)
  def real(); end

  sig.returns(Numeric)
  def real?(); end

  sig.returns([Numeric, Numeric])
  def rect(); end

  sig.returns([Numeric, Numeric])
  def rectangular(); end

  sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def remainder(arg0); end

  sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0); end

  sig(
      arg0: Symbol,
  )
  .returns(TypeError)
  def singleton_method_added(arg0); end

  sig(
      arg0: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  sig(
      arg0: Numeric,
  )
  .returns(Enumerator[Numeric])
  sig(
      arg0: Numeric,
      arg1: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  sig(
      arg0: Numeric,
      arg1: Numeric,
  )
  .returns(Enumerator[Numeric])
  def step(arg0, arg1=_, &blk); end

  sig.returns(Complex)
  def to_c(); end

  sig.returns(Integer)
  def to_int(); end

  sig.returns(Integer)
  def truncate(); end

  sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
