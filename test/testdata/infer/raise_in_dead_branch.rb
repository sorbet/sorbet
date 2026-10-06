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
def only_first_raise_is_allowed(x)
  unless true
    raise ArgumentError, "First failure"
    raise RuntimeError, "Unexpected value: #{x}" # error: This code is unreachable
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: String, y: Integer).void}
def only_first_raise_absurd_pair_is_checked(x, y)
  unless true
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
    raise RuntimeError, "Unexpected value: #{y}" # error: This code is unreachable
    T.absurd(y)
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
    T.absurd(x)
  end
end

sig {params(x: String).void}
def unrelated_code_after_live_raise_still_errors(x)
  raise ArgumentError, "Unexpected value: #{x}"
  puts(x) # error: This code is unreachable
  T.absurd(x)
end

sig {params(x: String, y: Integer).void}
def only_first_absurd_after_live_raise_is_checked(x, y)
  raise ArgumentError, x
  T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  T.absurd(y)
  puts(y) # error: This code is unreachable
end

sig {params(x: T.any(Integer, String)).void}
def exhaustive_dead_branch_multiple_predecessors(x)
  if x.is_a?(Integer) || x.is_a?(String)
  else
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x)
  end
end

sig {params(x: T.any(Integer, String), y: Symbol).void}
def non_exhaustive_dead_branch_multiple_predecessors(x, y)
  if x.is_a?(Integer) || x.is_a?(String)
  else
    raise ArgumentError, "Unexpected value: #{y}"
    T.absurd(y) # error: Control flow could reach `T.absurd` because the type `Symbol` wasn't handled
  end
end

sig {params(x: T.any(Integer, String, Symbol)).void}
def non_exhaustive_live_branch_multiple_predecessors(x)
  if x.is_a?(Integer) || x.is_a?(String)
  else
    raise ArgumentError, "Unexpected value: #{x}"
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Symbol` wasn't handled
  end
end

sig {params(x: String).returns(ArgumentError)}
def make_error(x)
  ArgumentError.new(x)
end

sig {params(x: String).void}
def raise_argument_call_in_dead_branch(x)
  unless true
    raise make_error(x)
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: Integer).void}
def raise_integer_argument_call_in_dead_branch(x)
  unless true
    raise make_error(x) # error: Expected `String` but found `Integer` for argument `x`
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Integer` wasn't handled
  end
end

sig {params(x: Integer).void}
def nested_raise_argument_call_in_dead_branch(x)
  unless true
    raise make_error(x.to_s)
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Integer` wasn't handled
  end
end

sig {params(x: String).void}
def unrelated_argument_errors_in_dead_branch_are_skipped(x)
  unless true
    make_error(123) # error: This code is unreachable
    raise make_error(x)
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
end

sig {params(x: String).void}
def paired_raise_is_reset_between_blocks(x)
  unless true
    raise make_error(x)
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `String` wasn't handled
  end
  unless true
    T.absurd(x)
  end
end
