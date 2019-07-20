# typed: true

extend T::Sig

sig {params(x: T.any(Integer, String)).void}
def normal_usage(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    T.impossible(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def forgotten_case(x)
  case x
  when Integer
    puts x
  else
    # TODO(jez) design better error messages; include type in all these error assertions
    T.impossible(x) # error: Control flow reached `T.impossible`
  end
end

sig {params(x: T.any(Integer, String)).void}
def got_all_cases_but_untyped(x)
  y = T.unsafe(x)
  case y
  when Integer
    puts y
  when String
    puts y
  else
    # TODO(jez) design better error messages; include type in all these error assertions
    # TODO(jez) might want to make this error message better
    T.impossible(y) # error: Control flow reached `T.impossible`
  end
end

sig {params(x: T.any(Integer, String, T::Array[Integer])).void}
def forgotten_two_cases(x)
  case x
  when Integer
    puts x
  else
    # TODO(jez) design better error messages; include type in all these error assertions
    T.impossible(x) # error: Control flow reached `T.impossible`
  end
end

sig {params(x: T.any(String, T::Array[Integer])).void}
def missing_case_with_generic(x)
  case x
  when Array
    puts x
  else
    # TODO(jez) design better error messages; include type in all these error assertions
    T.impossible(x) # error: Control flow reached `T.impossible`
  end
end

sig {params(x: T.any(T::Array[String], T::Array[Integer])).void}
def ok_when_generic_cases_overlap(x)
  case x
  when Array
    puts x
  else
    T.impossible(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def dead_code_before(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    puts 1 # error: This code is unreachable
    T.impossible(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def dead_code_after(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    T.impossible(x)
    puts 1 # error: This code is unreachable
  end
end

sig {params(x: T.any(Integer, String)).void}
def cant_alias_local_variables(x)
  case x
  when Integer
    puts x
  when String
    puts x
  else
    # Hello future Sorbet contributor.
    # It's not that we MUST have an error here, but rather that we were fine
    # with it, and couldn't see a simple solution that avoided emitting one.
    y = x
    #   ^ error: This code is unreachable
    T.impossible(y)
  end
end

sig {params(x: T.any(Integer, String)).void}
def normal_usage_with_isa(x)
  if x.is_a?(Integer)
    puts x
  elsif x.is_a?(String)
    puts x
  else
    T.impossible(x)
  end
end

sig {params(x: T.any(Integer, String)).void}
def missing_case_with_isa(x)
  if x.is_a?(Integer)
    puts x
  else
    # TODO(jez) Type in error message here
    T.impossible(x) # error: Control flow reached `T.impossible`
  end
end

sig {params(x: Integer).void}
def enforce_something_is_always_non_nil(x)
  if !x.nil?
    puts x
  else
    T.impossible(x)
  end
end

sig {params(x: Integer).void}
def error_when_predicate_always_true(x)
  if !x.nil?
    puts 1 # not a dead code error, because it's always true!
    T.impossible(x) # error: Control flow reached `T.impossible`
  end
end

# --- trying to subvert normal usage ------------------------------------------

def only_impossible_1
  T.impossible(T.let(T.unsafe(nil), T.noreturn)) # error: This code is unreachable
end

def only_impossible_2
  temp1 = T.let(T.unsafe(nil), T.noreturn)
  T.impossible(temp1) # error: This code is unreachable
end

sig {params(x: T.noreturn).returns(T.noreturn)}
def cant_call_only_impossible(x)
  T.impossible(x) # error: This code is unreachable
end

