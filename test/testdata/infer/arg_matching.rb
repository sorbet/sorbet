class TestArgs
  def required(a, b)
  end

  def call_required
    required(1) # error: not enough arguments
    required(1, 2)
    required(1, 2, 3) # error: too many arguments
  end

  def optional(a, b=1)
  end

  def call_optional
    optional(1)
    optional(1, 2)
    optional(1, 2, 3) # error: too many arguments
  end

  standard_method(
    {
      a: Integer,
      b: Integer,
    }, returns: NilType)
  def kwarg(a, b:)
  end

  def call_kwarg
    kwarg(1, 2) # error: too many arguments
    kwarg(1, b: 2)
    kwarg(1, {}) # error: missing argument b
    kwarg(1, b: "hi") # error:  Argument `b' does not match expected type.
  end
end
