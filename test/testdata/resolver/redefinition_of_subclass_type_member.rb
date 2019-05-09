# typed: true

class Foo
  extend T::Generic

  V = type_member
  K = type_member

  sig {params(k: K, v: V).returns(K)}
  def foo(k, v)
    k
  end
end

class Bar < Foo
  K = Bar[String,String].new.foo('a', 2)
end
