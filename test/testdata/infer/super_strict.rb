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
    super
  # ^^^^^ error: Method `foo` does not exist on ancestors of `A`
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
        # ^ error: Too many arguments provided for method `A#foo`. Expected: `0`, got: `1
    super
    super(1)
        # ^ error: Too many arguments provided for method `A#foo`. Expected: `0`, got: `1`
    T.reveal_type(super()) # error: Revealed type: `Integer`
  end

  sig { params(x: String).returns(Integer) }
  def bar(x)
        # ^ error: Expected `Integer` but found `String` for argument `x`
    super() # error: Not enough arguments provided for method `A#bar`. Expected: `1`, got: `0`
    super("a")
        # ^^^ error: Expected `Integer` but found `String("a")` for argument `x`
    super
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
