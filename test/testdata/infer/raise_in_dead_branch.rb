# typed: true

extend T::Sig

sig {params(x: String).void}
def non_exhaustive_dead_branch(x)
  unless true
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: T.any(Integer, String)).void}
def exhaustive_dead_branch(x)
  case x
  when Integer
  when String
  else
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def unresolved_raise_constant_still_errors(x)
  case x
  when Integer
  when String
  else
    raise NotFoundConstant # error: Unable to resolve constant
    T.absurd(x)
  end
end

sig {params(x: String).void}
def unrelated_dead_code_still_errors(x)
  unless true
    puts(x) # error: This code is unreachable
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: T.any(Integer, String)).void}
def non_exhaustive_live_branch(x)
  case x
  when Integer
  else
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {void}
def unpaired_raise_still_errors
  unless true
    raise ArgumentError, "Unexpected value" # error: This code is unreachable
  end
end

sig {params(x: String).void}
def only_closest_raise_is_allowed(x)
  unless true
    raise ArgumentError, "First failure" # error: This code is unreachable
    raise RuntimeError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: String, y: Integer).void}
def multiple_raise_absurd_pairs_are_allowed(x, y)
  unless true
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
    raise RuntimeError, "Unexpected value: #{y}"
    T.absurd(y) # error: Control flow could reach `T.absurd` because the type `Integer` wasn't handled
  end
end

sig {params(x: T.any(Integer, String)).void}
def raise_after_absurd_still_errors(x)
  case x
  when Integer
  when String
  else
    T.absurd(x)
    raise ArgumentError, "Unexpected value: #{x}" # error: This code is unreachable
  end
end
