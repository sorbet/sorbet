# typed: true

class Base1
  extend T::Generic
  Elem = type_member
end

class Base2
  extend T::Generic
  Elem = type_member
end
class Child2 < Base2
  extend T::Generic
  Elem = type_member
end

class Test
  extend T::Sig

  sig {params(x: Base1[T.untyped]).void}
  def test1(x)
    if x.is_a? Base2
      puts x # error: This code is unreachable
      T.absurd(x)
    end

    T.reveal_type(x) # error: Revealed type: `Base1[T.untyped]`

    if x.is_a? Child2
      puts x # error: This code is unreachable
      T.absurd(x)
    end

    T.reveal_type(x) # error: Revealed type: `Base1[T.untyped]`
  end
end
