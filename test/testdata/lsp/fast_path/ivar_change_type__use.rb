# typed: true

class Child < Parent
  extend T::Sig

  sig {returns(Integer)}
  def x
    @x
  end
end
