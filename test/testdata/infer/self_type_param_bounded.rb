# typed: strict

class Animal; end
class Cat < Animal; end
class Serval < Cat; end

class A
  extend T::Sig
  extend T::Generic

  T1 = type_member {{upper: Animal, lower: Cat}}

  sig {params(x: T1).void}
  def test1(x)
    T.cast(x, Animal) # error: `T.cast` is useless
  end
end

class B
  extend T::Sig
  extend T::Generic

  T1 = type_member {{upper: Object}}
  T2 = type_member {{lower: Object}}

  sig {params(x: T1).void}
  def test1(x)
    T.reveal_type(test2(x)) # error: Revealed type: `B::T2`
  end

  sig {params(x: T2).returns(T2)}
  def test2(x)
    x
  end
end

module C
  extend T::Sig
  extend T::Generic

  Out = type_member(:out) {{upper: Animal}}
  In = type_member(:in) {{lower: Cat}}

  sig {void}
  def test1
    C.test1(self)
  end

  sig {params(x: C[Animal,Cat]).void}
  def self.test1(x)
  end
end
