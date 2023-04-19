# typed: true

module M
  def foo
    T.reveal_type(super) # error: Revealed type: T.untyped
  end
end

class A
  extend T::Sig

  sig { returns(Integer) }
  def foo
    super # some sort of error?
    1
  end

  sig { params(x: Integer).returns(Integer) }
  def bar(x)
    1
  end
end

class B < A
  sig { params(x: Integer).returns(Integer) }
  def foo(x)
    super # some sort of error?
    super(1) # some sort of error?
    T.reveal_type(super()) # error: Revealed type: Integer
  end

  sig { params(x: Integer).returns(Integer) }
  def bar(x)
    super() # some sort of error?
    super("a") # some sort of error?
  end
end
