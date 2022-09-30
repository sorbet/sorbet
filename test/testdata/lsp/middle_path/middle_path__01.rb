# typed: true

class A_01
  extend T::Sig

  sig {returns(Integer)}
  def foo()
    42
  end
end
