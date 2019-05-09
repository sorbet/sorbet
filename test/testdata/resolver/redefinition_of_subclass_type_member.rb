# typed: true
# disable-fast-path: true

class Foo
  extend T::Generic
  extend T::Sig

  V = type_member
  K = type_member

  sig {params(k: K, v: V).returns(K)}
  def foo(k, v)
    k
  end
end

class Bar < Foo # error: Type `V` declared by parent `Foo` must be declared again
  K = Bar[String,String].new.foo('a', 2)
# ^ error: Type variable `K` needs to be declared as `= type_member(SOMETHING)`
    # ^^^^^^^^^^^^^^^^^^ error: Wrong number of type parameters
end
