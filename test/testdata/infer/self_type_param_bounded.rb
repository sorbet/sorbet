# typed: strict

class Animal; end
class Cat < Animal; end

class A
  extend T::Sig
  extend T::Generic

  X = type_template(upper: Animal)

  sig {params(x: X).void}
  def self.useless_cast(x)
    x = T.cast(x, Animal) # error: Useless cast
  end

  Y = type_member(upper: Cat)

  sig {params(y: Y).void}
  def useless_cast(y)
    y = T.cast(y, Cat) # error: Useless cast
  end
end
