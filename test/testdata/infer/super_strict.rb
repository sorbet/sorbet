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

  sig { params(blk: T.proc.void).void }
  def takes_block(&blk)
    yield
  end

  sig { params(x: Integer).void }
  def required_kwargs(x:)
  end
end

class B < A
  extend T::Sig

  include M

  sig { params(x: Integer).returns(Integer) }
  def foo(x)
    super
    #    ^ error: Too many arguments provided for method `A#foo`. Expected: `0`, got: `1
    super(1)
        # ^ error: Too many arguments provided for method `A#foo`. Expected: `0`, got: `1`
    T.reveal_type(super()) # error: Revealed type: `Integer`
  end

  sig { params(x: String).returns(Integer) }
  def bar(x)
    super() # error: Not enough arguments provided for method `A#bar`. Expected: `1`, got: `0`
    super("a")
        # ^^^ error: Expected `Integer` but found `String("a")` for argument `x`
    super
    #    ^ error: Expected `Integer` but found `String` for argument `x`
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

  sig { void }
  def takes_block
    super
    #    ^ error: `takes_block` requires a block parameter, but no block was passed
  end

  sig { params(opts: T.untyped).void}
  def required_kwargs(**opts)
    super
    #    ^ error: the method has required keyword parameters
  end
end
