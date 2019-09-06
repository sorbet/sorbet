# typed: true

class Base1
  extend T::Generic
  Elem = type_member
end

class Base2
  extend T::Generic
  Elem = type_member
end
class Child < Base2
  extend T::Generic
  Elem = type_member
end

class Test
  extend T::Sig

  sig {params(x: Base1[T.untyped]).void}
  def test1(x)
    if x.is_a? Base2
      T.absurd(x)
    end

    if x.is_a? Child
      T.absurd(x)
    end
  end
end
