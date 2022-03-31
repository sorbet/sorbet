# typed: true

class A
  def foo; end
end

class B
  def foo(x); end
end

a = T.let(A.new, T.any(A, B))
  a.foo # error: Not enough arguments provided for method `B#foo` on `B` component of `T.any(A, B)`. Expected: `1`, got: `0`
