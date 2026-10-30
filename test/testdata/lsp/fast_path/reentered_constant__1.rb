# typed: true

class A
  extend T::Sig

  X = T.let(1, Integer)

  sig {returns(Integer)}
  def foo
    0
  end
end
