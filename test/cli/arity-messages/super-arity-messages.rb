# typed: strict

class Parent
  extend T::Sig

  sig {params(x: Integer).void}
  def unary1(x)
  end

  sig {params(x: Integer).void}
  def unary2(x)
    super
  end
end

class Child < Parent
  sig {void}
  def unary1
    super
  end

  sig {params(x: Integer, y: String).void}
  def unary2(x, y)
    super
  end
end
