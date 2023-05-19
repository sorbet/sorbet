# typed: true

module M
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
    1
  end
end

class B < A
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
end
