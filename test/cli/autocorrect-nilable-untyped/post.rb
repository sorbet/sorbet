# typed: true

class A < T::Struct
  extend T::Sig

  prop :a, T.untyped, default: nil
  prop :b, T.untyped, default: "Hello"
  const :c, T.untyped, default: nil
  const :d, T.untyped, default: "Hello"

  sig {params(x: T.untyped).returns(T.untyped)}
  def test1(x)
    x
  end

  sig {returns(T.untyped)}
  def test2
    res = T.let('hi', T.untyped)
    res
  end

  X = T.type_alias{T.untyped}
  sig {returns(T.nilable(X))}
  def test3
    nil
  end

end

class B
  extend T::Sig
  extend T::Generic

  Elem = type_member(upper: T.untyped)

end
