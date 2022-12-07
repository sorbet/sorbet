# typed: strict

module I
  extend T::Sig
  extend T::Helpers
  interface!

  sig do
    abstract.params(
      a: Integer,
      b: Integer,
      c: Integer,
      d: Integer,
      e: Integer,
      f: Integer,
      g: Integer,
      h: Integer,
      i: Integer,
      j: Integer,
      k: Integer,
      l: Integer,
      m: Integer,
      n: Integer
    ).returns(Integer)
  end
  def foo(
    a:,
    b:,
    c:,
    d:,
    e:,
    f:,
    g: 10,
    h: 10,
    i: 10,
    j: 10,
    k: 10,
    l: 10,
    m: 10,
    n: 10
  )
  end
end

class C
  extend T::Sig
  include I

  sig do
    override.params(
      a: Integer,
      b: Integer,
      c: Integer,
      d: Integer,
      e: Integer,
      f: Integer,
      g: Integer,
      h: Integer,
      i: Integer,
      j: Integer,
      k: Integer,
      l: Integer,
      m: Integer,
      n: Integer
    ).returns(Integer)
  end
  def foo(
    a:,
    b:,
    c:,
    d:,
    e:,
    f:,
    g: 10,
    h: 10,
    i: 10,
    j: 10,
    k: 10,
    l: 10,
    m: 10,
    n: 10
  )
    10
  end
end
