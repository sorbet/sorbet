# typed: false
# Not typed; Demonstrating lax `sig`-parsing in untyped code

class A
  sig do
    params(
      a: T.enum([1,2]),
      b: T.any(*[Integer, String])
    ).returns(T.untyped)
  end
  def f(a, b)
  end
end
