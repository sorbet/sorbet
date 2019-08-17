# typed: true

class A
  extend T::Generic
  X = type_member
end

class B < A
  extend T::Generic
  X = type_member(fixed: String)
end

class Test
  extend T::Sig

  sig {params(x: A[T.untyped]).void}
  def test1(x)
    case x
    when B
      T.reveal_type(x) # error: Revealed type: `B`
    when A
      T.reveal_type(x) # error: Revealed type: `A[T.untyped]`
    end
  end

  sig {params(x: A[String]).void}
  def test2(x)
    case x
    when B
      T.reveal_type(x) # error: Revealed type: `B`
    when A
      T.reveal_type(x) # error: Revealed type: `A[String]`
    end
  end

  sig {params(x: T.nilable(A[Integer])).void}
  def test3(x)
    case x
    when B
      T.reveal_type(x) # error: Revealed type: `T.all(B, A[Integer])`
    when A
      T.reveal_type(x) # error: Revealed type: `A[Integer]`
    end
  end

  sig {params(x: T.nilable(A[String])).void}
  def test4(x)
    case x
    when B
      T.reveal_type(x) # error: Revealed type: `B`
    when A
      T.reveal_type(x) # error: Revealed type: `A[String]`
    end
  end

  sig {params(x: A[String]).void}
  def test5(x)
    if x.is_a? B
      T.reveal_type(x) # error: Revealed type: `B`
    elsif x.is_a? A
      T.reveal_type(x) # error: Revealed type: `A[String]`
    end
  end

  sig {params(x: T.nilable(A[String])).void}
  def test6(x)
    if x.is_a? B
      T.reveal_type(x) # error: Revealed type: `B`
    elsif x.is_a? A
      T.reveal_type(x) # error: Revealed type: `A[String]`
    end
  end

end
