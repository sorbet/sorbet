# typed: true

class Child < Parent
  extend T::Sig

  sig {returns(Integer)}
  def y
    @@y
  end
end
