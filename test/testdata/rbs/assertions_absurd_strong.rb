# typed: strong
# enable-experimental-rbs-comments: true

# `raise #: absurd(x)` must not introduce calls on `T.untyped`.

class A
  #: (Integer | String) -> void
  def exhaustive(x)
    case x
    when Integer
    when String
    else
      raise ArgumentError, "Unexpected value" #: absurd(x)
    end
  end

  #: (Integer | String) -> void
  def not_exhaustive(x)
    case x
    when Integer
    else
      raise ArgumentError, "Unexpected value" #: absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
    end
  end
end
