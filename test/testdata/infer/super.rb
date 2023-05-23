# typed: strict

module M
  extend T::Sig

  sig { returns(Integer) }
  def baz
    T.reveal_type(super) # error: Revealed type: `T.untyped`
  end
end

class A
  extend T::Sig

  sig { returns(Integer) }
  def foo
    super # error: 
    1
  end

  sig { params(x: Integer).returns(Integer) }
  def bar(x)
    x
  end

  sig { params(x: Integer).returns(Integer) }
  def quz(x)
    x
  end
end

class B < A
  extend T::Sig

  include M

  sig { params(x: Integer).returns(Integer) }
  def foo(x)
    super # error: 
    super(1) # error: 
    T.reveal_type(super()) # error: Revealed type: `Integer`
  end

  sig { params(x: String).returns(Integer) }
  def bar(x)
    super() # error: 
    super("a") # error: 
    super # error: 
    super(1)
  end

  sig { returns(Integer) }
  def baz
    T.reveal_type(super) # error: Revealed type: `Integer`
  end

  sig { params(x: Integer).returns(Integer) }
  def quz(x)
    super
  end
end
