# typed: true

class A < T::Struct
  extend T::Sig

  prop :a, T.nilable(T.untyped)
  prop :b, T.nilable(T.untyped), default: "Hello"
  const :c, T.nilable(T.untyped)
  const :d, T.nilable(T.untyped), default: "Hello"

  sig {params(x: T.nilable(T.untyped)).returns(T.nilable(T.untyped))}
  def test1(x)
    x
  end

  sig {returns(T.nilable(T.untyped))}
  def test2
    res = T.let('hi', T.nilable(T.untyped))
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

  Elem = type_member {{upper: T.nilable(T.untyped)}}

end
