class TestArgs
  def any; end

  standard_method({}, returns: Hash)
  def a_hash; end

  def required(a, b)
  end

  def call_required
    required(1) # error: Not enough arguments
    required(1, 2)
    required(1, 2, 3) # error: Too many arguments
  end

  def optional(a, b=1)
  end

  def call_optional
    optional(1)
    optional(1, 2)
    optional(1, 2, 3) # error: Too many arguments
  end

  standard_method(
    {
      a: Integer,
      b: Integer,
    }, returns: NilClass)
  def kwarg(a, b:)
  end

  def call_kwarg
    # "too many arguments" and "missing keyword argument b"
    kwarg(1, 2) # error: MULTI

    kwarg(1) # error: Missing required keyword argument b

    kwarg(1, b: 2)
    kwarg(1, {}) # error: Missing required keyword argument b
    kwarg(1, b: "hi") # error: Argument `b' does not match expected type.
    kwarg(1, any)
    kwarg(1, a_hash) # error: Passing an untyped hash
  end

  standard_method(
    {
      x: Integer
    }, returns: NilClass)
  def repeated(*x)
  end

  def call_repeated
    repeated
    repeated(1, 2, 3)
    repeated(1, "hi") # error: Argument `x' does not match expected type.

    # We error on each incorrect argument
    repeated("hi", "there") # error: MULTI
  end
end
