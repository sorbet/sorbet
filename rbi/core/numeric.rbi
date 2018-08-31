# typed: true
class Numeric < Object
  include Comparable

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def %(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def +(arg0); end

  Sorbet.sig.returns(Numeric)
  def +@(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def -(arg0); end

  Sorbet.sig(
    arg0: Numeric,
  )
  .returns(Numeric)
  def *(arg0); end

  Sorbet.sig(
    arg0: Numeric,
  )
  .returns(Numeric)
  def /(arg0); end

  Sorbet.sig.returns(Numeric)
  def -@(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def <=(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def <=>(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def >=(arg0); end

  Sorbet.sig.returns(Numeric)
  def abs(); end

  Sorbet.sig.returns(Numeric)
  def abs2(); end

  Sorbet.sig.returns(Numeric)
  def angle(); end

  Sorbet.sig.returns(Numeric)
  def arg(); end

  Sorbet.sig.returns(Integer)
  def ceil(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def coerce(arg0); end

  Sorbet.sig.returns(Numeric)
  def conj(); end

  Sorbet.sig.returns(Numeric)
  def conjugate(); end

  Sorbet.sig.returns(Integer)
  def denominator(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Integer)
  def div(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns([Numeric, Numeric])
  def divmod(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(TrueClass, FalseClass))
  def eql?(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def fdiv(arg0); end

  Sorbet.sig.returns(Integer)
  def floor(); end

  Sorbet.sig.returns(Complex)
  def i(); end

  Sorbet.sig.returns(Numeric)
  def imag(); end

  Sorbet.sig.returns(Numeric)
  def imaginary(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def integer?(); end

  Sorbet.sig.returns(Numeric)
  def magnitude(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def modulo(arg0); end

  Sorbet.sig.returns(T.nilable(T.self_type))
  def nonzero?(); end

  Sorbet.sig.returns(Integer)
  def numerator(); end

  Sorbet.sig.returns(Numeric)
  def phase(); end

  Sorbet.sig.returns([Numeric, Numeric])
  def polar(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def quo(arg0); end

  Sorbet.sig.returns(Numeric)
  def real(); end

  Sorbet.sig.returns(Numeric)
  def real?(); end

  Sorbet.sig.returns([Numeric, Numeric])
  def rect(); end

  Sorbet.sig.returns([Numeric, Numeric])
  def rectangular(); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(T.any(Integer, Float, Rational, BigDecimal))
  def remainder(arg0); end

  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Numeric)
  def round(arg0); end

  Sorbet.sig(
      arg0: Symbol,
  )
  .returns(TypeError)
  def singleton_method_added(arg0); end

  Sorbet.sig(
      arg0: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: Numeric,
  )
  .returns(Enumerator[Numeric])
  Sorbet.sig(
      arg0: Numeric,
      arg1: Numeric,
      blk: T.proc(arg0: Numeric).returns(BasicObject),
  )
  .returns(Numeric)
  Sorbet.sig(
      arg0: Numeric,
      arg1: Numeric,
  )
  .returns(Enumerator[Numeric])
  def step(arg0, arg1=T.unsafe(nil), &blk); end

  Sorbet.sig.returns(Complex)
  def to_c(); end

  Sorbet.sig.returns(Float)
  def to_f(); end

  Sorbet.sig.returns(Integer)
  def to_i(); end

  Sorbet.sig.returns(Integer)
  def to_int(); end

  Sorbet.sig.returns(Integer)
  def truncate(); end

  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def zero?(); end
end
