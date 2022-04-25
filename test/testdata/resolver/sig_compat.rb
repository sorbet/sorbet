# typed: false


class A
  sig do
    params(
      b: T.deprecated_enum([1,2]),
      c: T.any(*[Integer, String]) # error: splats cannot be used in types
    ).returns(T.untyped)
  end
  def f(b, c)
  end
end
