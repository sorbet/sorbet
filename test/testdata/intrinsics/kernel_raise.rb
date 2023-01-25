# typed: true
extend T::Sig

class MyError < StandardError
  extend T::Sig

  sig { params(input: String).void }
  def initialize(input:)
    @input = input
  end
end

class MultipleRequired < StandardError
  extend T::Sig

  sig { params(input: String, another: Integer).void }
  def initialize(input, another)
  end
end

class DefinesToException
  extend T::Sig

  sig { params(arg0: String).void }
  def initialize(arg0)
  end

  sig { returns(Exception) }
  def exception
    TypeError.new("hello")
  end
end

class DoesNotDefineToException
  extend T::Sig

  sig { params(arg0: String).void }
  def initialize(arg0)
  end
end

sig {params(cls: T.any(T.class_of(TypeError), T.class_of(MyError))).void}
def example(cls)
  0.times do
    raise MyError
    #            ^ error: Missing required keyword argument `input` for method `MyError#initialize`
  end

  0.times do
    raise cls
    #        ^ error: Missing required keyword argument `input` for method `MyError#initialize`
  end

  0.times do
    raise MyError, "arg"
    #              ^^^^^ error: Missing required keyword argument `input` for method `MyError#initialize`
    #              ^^^^^ error: Too many positional arguments provided for method `MyError#initialize`. Expected: `0`, got: `1`
  end

  0.times do
    raise cls, "arg"
    #          ^^^^^ error: Missing required keyword argument `input` for method `MyError#initialize`
    #          ^^^^^ error: Too many positional arguments provided for method `MyError#initialize`. Expected: `0`, got: `1`
  end

  0.times do
    raise MyError.new
    #                ^ error: Missing required keyword argument `input` for method `MyError#initialize`
  end

  0.times do
    raise T.unsafe(MyError)
  end

  0.times do
    # If this did not define `exception`, it would dispatch to Exception.exception,
    # which forwards to `new` and then `initialize`, which requires a single pos arg
    # that isn't provided here.
    raise DefinesToException
  end

  0.times do
    # This doesn't define `exception`, but we don't currently detect that, (and
    # probably can't, because of subtyping), opting instead to skip the intrinsic
    raise DoesNotDefineToException
  end

  0.times do
    raise TypeError, "one", "two"
    #                       ^^^^^ error: Expected `T.nilable(T::Array[String])` but found `String("two")` for argument `arg2`
  end

  0.times do
    raise MultipleRequired, "one", "two"
    #                              ^^^^^ error: Expected `T.nilable(T::Array[String])` but found `String("two")` for argument `arg2`
    #                       ^^^^^ error: Not enough arguments provided for method `MultipleRequired#initialize`. Expected: `2`, got: `1`
  end

  0.times do
    fail MyError
    #           ^ error: Missing required keyword argument `input` for method `MyError#initialize`
  end
end
