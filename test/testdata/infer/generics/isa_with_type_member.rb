# typed: true

class Parent
  extend T::Generic
  X = type_member
end

class Child < Parent
  extend T::Generic
  X = type_member(fixed: String)
end

class Test
  extend T::Sig

  sig {params(x: Parent[T.untyped]).void}
  def test1(x)
    case x
    when Child
      T.reveal_type(x) # error: Revealed type: `Child`
    when Parent
      T.reveal_type(x) # error: Revealed type: `Parent[T.untyped]`
    end
  end

  sig {params(x: Parent[String]).void}
  def test2(x)
    case x
    when Child
      T.reveal_type(x) # error: Revealed type: `Child`
    when Parent
      T.reveal_type(x) # error: Revealed type: `Parent[String]`
    end
  end

  sig {params(x: T.nilable(Parent[Integer])).void}
  def test3(x)
    case x
    when Child
      T.reveal_type(x) # error: Revealed type: `T.all(Child, Parent[Integer])`
    when Parent
      T.reveal_type(x) # error: Revealed type: `Parent[Integer]`
    end
  end

  sig {params(x: T.nilable(Parent[String])).void}
  def test4(x)
    case x
    when Child
      T.reveal_type(x) # error: Revealed type: `Child`
    when Parent
      T.reveal_type(x) # error: Revealed type: `Parent[String]`
    end
  end

  sig {params(x: Parent[String]).void}
  def test5(x)
    if x.is_a? Child
      T.reveal_type(x) # error: Revealed type: `Child`
    elsif x.is_a? Parent
      T.reveal_type(x) # error: Revealed type: `Parent[String]`
    end
  end

  sig {params(x: T.nilable(Parent[String])).void}
  def test6(x)
    if x.is_a? Child
      T.reveal_type(x) # error: Revealed type: `Child`
    elsif x.is_a? Parent
      T.reveal_type(x) # error: Revealed type: `Parent[String]`
    end
  end

end


class Nil
  extend T::Generic
  Elem = type_member(fixed: T.noreturn)
                   # ^^^^^ error: This code is unreachable
end

class Cons
  extend T::Generic
  Elem = type_member
end

extend T::Sig

sig {params(x: T.any(Nil, Cons[Integer])).void}
def foo(x)
  if x.is_a? Cons
    T.reveal_type(x) # error: Revealed type: `Cons[Integer]`
  elsif x.is_a? Nil
    T.reveal_type(x) # error: Revealed type: `Nil`
  else
    T.absurd(x)
  end
end
