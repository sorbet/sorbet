# typed: true

class MyClass
  extend T::Sig

  A = T.let(1, Integer)

  sig {params(x: Integer).returns(Integer)}
  def frob(x)
    x + A
  end
end
