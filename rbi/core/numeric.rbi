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
